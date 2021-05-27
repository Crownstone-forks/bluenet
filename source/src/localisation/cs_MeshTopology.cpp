/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Apr 30, 2021
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <ble/cs_Nordic.h>
#include <localisation/cs_MeshTopology.h>
#include <storage/cs_State.h>
#include <util/cs_Rssi.h>
#include <uart/cs_UartHandler.h>

#define LOGMeshTopologyInfo    LOGi
#define LOGMeshTopologyDebug   LOGd
#define LOGMeshTopologyVerbose LOGvv

cs_ret_code_t MeshTopology::init() {

	State::getInstance().get(CS_TYPE::CONFIG_CROWNSTONE_ID, &_myId, sizeof(_myId));

	_neighbours = new (std::nothrow) neighbour_node_t[MAX_NEIGHBOURS];
	if (_neighbours == nullptr) {
		return ERR_NO_SPACE;
	}
	reset();
	listen();

#if BUILD_MESH_TOPOLOGY_RESEARCH == 1
	_research.init();
#endif

	return ERR_SUCCESS;
}

void MeshTopology::reset() {
	LOGMeshTopologyInfo("Reset");

	// Remove stored neighbours.
	_neighbourCount = 0;

	// Let everyone first send a noop, and then the first result.
	_sendNoopCountdown = 1;
	_sendCountdown = 2;
	_fastIntervalCountdown = FAST_INTERVAL_TIMEOUT_SECONDS;
}

cs_ret_code_t MeshTopology::getMacAddress(stone_id_t stoneId) {
	LOGMeshTopologyInfo("getMacAddress %u", stoneId);
	uint8_t index = find(stoneId);
	if (index == INDEX_NOT_FOUND) {
		LOGMeshTopologyInfo("Not a neighbour");
		return ERR_NOT_FOUND;
	}

	cs_mesh_model_msg_stone_mac_t request;
	request.type = 0;

	cs_mesh_msg_t meshMsg;
	meshMsg.type = CS_MESH_MODEL_TYPE_STONE_MAC;
	meshMsg.flags.flags.broadcast = false;
	meshMsg.flags.flags.reliable = true;
	meshMsg.flags.flags.useKnownIds = false;
	meshMsg.flags.flags.noHops = false; // No reliable msgs without hops yet.
	meshMsg.reliability = 3; // Low timeout, we expect a result quickly.
	meshMsg.urgency = CS_MESH_URGENCY_LOW;
	meshMsg.idCount = 1;
	meshMsg.targetIds = &stoneId;
	meshMsg.payload = reinterpret_cast<uint8_t*>(&request);
	meshMsg.size = sizeof(request);

	event_t event(CS_TYPE::CMD_SEND_MESH_MSG, &meshMsg, sizeof(meshMsg));
	event.dispatch();
	LOGMeshTopologyInfo("Sent mesh msg retCode=%u", event.result.returnCode);
	if (event.result.returnCode != ERR_SUCCESS) {
		return event.result.returnCode;
	}

	return ERR_WAIT_FOR_SUCCESS;
}


void MeshTopology::add(stone_id_t id, int8_t rssi, uint8_t channel) {
	if (id == 0 || id == _myId) {
		return;
	}
//	auto compressedRssiData = compressRssi(rssi,channel);

	uint8_t index = find(id);
	if (index == INDEX_NOT_FOUND) {
		if (_neighbourCount < MAX_NEIGHBOURS) {
			// Init RSSI
			_neighbours[_neighbourCount].rssiChannel37 = RSSI_INIT;
			_neighbours[_neighbourCount].rssiChannel38 = RSSI_INIT;
			_neighbours[_neighbourCount].rssiChannel39 = RSSI_INIT;
			updateNeighbour(_neighbours[_neighbourCount], id, rssi, channel);
			_neighbourCount++;
		}
		else {
			LOGw("Can't add id=%u", id);
		}
	}
	else {
		updateNeighbour(_neighbours[index], id, rssi, channel);
	}
}

void MeshTopology::updateNeighbour(neighbour_node_t& node, stone_id_t id, int8_t rssi, uint8_t channel) {
	// TODO: averaging.
	node.id = id;
	switch (channel) {
		case 37: {
			node.rssiChannel37  = rssi;
			break;
		}
		case 38: {
			node.rssiChannel38  = rssi;
			break;
		}
		case 39: {
			node.rssiChannel39  = rssi;
			break;
		}
	}
	node.lastSeenSecondsAgo = 0;
}

uint8_t MeshTopology::find(stone_id_t id) {
	for (uint8_t index = 0; index < _neighbourCount; ++index) {
		if (_neighbours[index].id == id) {
			return index;
		}
	}
	return INDEX_NOT_FOUND;
}

void MeshTopology::getRssi(stone_id_t stoneId, cs_result_t& result) {
	uint8_t index = find(stoneId);
	if (index == INDEX_NOT_FOUND) {
		result.returnCode = ERR_NOT_FOUND;
		return;
	}

	// Simply use the first valid rssi.
	int8_t rssi = _neighbours[index].rssiChannel37;
	if (rssi == 0) {
		rssi = _neighbours[index].rssiChannel38;
	}
	if (rssi == 0) {
		rssi = _neighbours[index].rssiChannel39;
	}
	if (rssi == 0) {
		result.returnCode = ERR_NOT_AVAILABLE;
		return;
	}

	if (result.buf.len < sizeof(rssi)) {
		result.returnCode = ERR_BUFFER_TOO_SMALL;
		return;
	}
	// Copy to result buf.
	result.buf.data[0] = *reinterpret_cast<uint8_t*>(&rssi);
	result.dataSize = sizeof(rssi);
	result.returnCode = ERR_SUCCESS;
}

void MeshTopology::sendNoop() {
	LOGMeshTopologyDebug("sendNoop");
	TYPIFY(CMD_SEND_MESH_MSG) meshMsg;
	meshMsg.type = CS_MESH_MODEL_TYPE_CMD_NOOP;
	meshMsg.reliability = CS_MESH_RELIABILITY_LOWEST;
	meshMsg.urgency = CS_MESH_URGENCY_LOW;
	meshMsg.flags.flags.noHops = true;
	meshMsg.payload = nullptr;
	meshMsg.size = 0;

	event_t event(CS_TYPE::CMD_SEND_MESH_MSG, &meshMsg, sizeof(meshMsg));
	event.dispatch();
}

void MeshTopology::sendNext() {
	if (_neighbourCount == 0) {
		// Nothing to send.
		return;
	}

	// Make sure index is valid.
	if (_nextSendIndex >= _neighbourCount) {
		_nextSendIndex = 0;
	}

	auto& node = _neighbours[_nextSendIndex];
	LOGMeshTopologyDebug("sendNextMeshMessage index=%u id=%u lastSeenSecondsAgo=%u", _nextSendIndex, node.id, node.lastSeenSecondsAgo);

	cs_mesh_model_msg_neighbour_rssi_t meshPayload = {
			.type = 0,
			.neighbourId = node.id,
			.rssiChannel37 = node.rssiChannel37,
			.rssiChannel38 = node.rssiChannel38,
			.rssiChannel39 = node.rssiChannel39,
			.lastSeenSecondsAgo = node.lastSeenSecondsAgo
	};

	TYPIFY(CMD_SEND_MESH_MSG) meshMsg;
	meshMsg.type = CS_MESH_MODEL_TYPE_NEIGHBOUR_RSSI;
	meshMsg.reliability = CS_MESH_RELIABILITY_LOWEST;
	meshMsg.urgency = CS_MESH_URGENCY_LOW;
	meshMsg.flags.flags.noHops = false;
	meshMsg.payload = reinterpret_cast<uint8_t*>(&meshPayload);
	meshMsg.size = sizeof(meshPayload);

	event_t event(CS_TYPE::CMD_SEND_MESH_MSG, &meshMsg, sizeof(meshMsg));
	event.dispatch();

	// Also send over UART.
	sendRssiToUart(_myId, meshPayload);

	// Send next item in the list next time.
	_nextSendIndex++;
}

void MeshTopology::sendRssiToUart(stone_id_t receiverId, cs_mesh_model_msg_neighbour_rssi_t& packet) {
	LOGMeshTopologyDebug("sendRssiToUart receiverId=%u senderId=%u", receiverId, packet.neighbourId);
	mesh_topology_neighbour_rssi_uart_t uartMsg = {
			.type = 0,
			.receiverId = receiverId,
			.senderId = packet.neighbourId,
			.rssiChannel37 = packet.rssiChannel37,
			.rssiChannel38 = packet.rssiChannel38,
			.rssiChannel39 = packet.rssiChannel39,
			.lastSeenSecondsAgo = packet.lastSeenSecondsAgo
	};
	UartHandler::getInstance().writeMsg(UART_OPCODE_TX_NEIGHBOUR_RSSI, reinterpret_cast<uint8_t*>(&uartMsg), sizeof(uartMsg));
}

void MeshTopology::onNeighbourRssi(stone_id_t id, cs_mesh_model_msg_neighbour_rssi_t& packet) {
	// Send to UART.
	sendRssiToUart(id, packet);
}

cs_ret_code_t MeshTopology::onStoneMacMsg(stone_id_t id, cs_mesh_model_msg_stone_mac_t& packet, mesh_reply_t* reply) {
	switch (packet.type) {
		case 0: {
			LOGMeshTopologyInfo("Send mac address");

			if (reply == nullptr) {
				return ERR_BUFFER_UNASSIGNED;
			}
			if (reply->buf.len < sizeof(cs_mesh_model_msg_stone_mac_t)) {
				return ERR_BUFFER_TOO_SMALL;
			}
			cs_mesh_model_msg_stone_mac_t* replyPacket = reinterpret_cast<cs_mesh_model_msg_stone_mac_t*>(reply->buf.data);
			replyPacket->type = 1;

			ble_gap_addr_t address;
			if (sd_ble_gap_addr_get(&address) != NRF_SUCCESS) {
				return ERR_UNSPECIFIED;
			}
			memcpy(replyPacket->mac, address.addr, MAC_ADDRESS_LEN);
			reply->type = CS_MESH_MODEL_TYPE_STONE_MAC;
			reply->dataSize = sizeof(cs_mesh_model_msg_stone_mac_t);
			break;
		}
		case 1: {
			LOGMeshTopologyInfo("Received mac address id=%u mac=%02X:%02X:%02X:%02X:%02X:%02X", id, packet.mac[5], packet.mac[4], packet.mac[3], packet.mac[2], packet.mac[1], packet.mac[0]);
			TYPIFY(EVT_MESH_TOPO_MAC_RESULT) result;
			result.stoneId = id;
			memcpy(result.macAddress, packet.mac, sizeof(result.macAddress));
			event_t event(CS_TYPE::EVT_MESH_TOPO_MAC_RESULT, &result, sizeof(result));
			event.dispatch();
			break;
		}
	}
	return ERR_SUCCESS;
}

void MeshTopology::onMeshMsg(MeshMsgEvent& packet, cs_result_t& result) {
	if (packet.type == CS_MESH_MODEL_TYPE_STONE_MAC) {
		cs_mesh_model_msg_stone_mac_t payload = packet.getPacket<CS_MESH_MODEL_TYPE_STONE_MAC>();
		result.returnCode = onStoneMacMsg(packet.srcAddress, payload, packet.reply);
	}
	if (packet.type == CS_MESH_MODEL_TYPE_NEIGHBOUR_RSSI) {
		cs_mesh_model_msg_neighbour_rssi_t payload = packet.getPacket<CS_MESH_MODEL_TYPE_NEIGHBOUR_RSSI>();
		onNeighbourRssi(packet.srcAddress, payload);
	}

	if (packet.hops != 0) {
		return;
	}
	add(packet.srcAddress, packet.rssi, packet.channel);
}

void MeshTopology::onTickSecond() {
	LOGMeshTopologyVerbose("onTickSecond nextSendIndex=%u", _nextSendIndex);
	print();
	[[maybe_unused]] bool change = false;
	for (uint8_t i = 0; i < _neighbourCount; /**/ ) {
		_neighbours[i].lastSeenSecondsAgo++;
		if (_neighbours[i].lastSeenSecondsAgo == TIMEOUT_SECONDS) {
			change = true;
			// Remove item, by shifting all items after this item.
			_neighbourCount--;
			for (uint8_t j = i; j < _neighbourCount; ++j) {
				_neighbours[j] = _neighbours[j + 1];
			}
			// Also change shift the next send index.
			if (_nextSendIndex > i) {
				_nextSendIndex--;
			}
		}
		else {
			i++;
		}
	}
	if (change) {
		LOGMeshTopologyVerbose("Result: nextSendIndex=%u", _nextSendIndex);
		print();
	}

	if (_sendCountdown != 0) {
		_sendCountdown--;
	}
	if (_sendCountdown == 0) {
		sendNext();
		// Even if we end up setting sendCountdown to 0, the next message will be sent next onTickSecond.
		if (_fastIntervalCountdown) {
			_sendCountdown = SEND_INTERVAL_SECONDS_PER_NEIGHBOUR_FAST / _neighbourCount;
		}
		else {
			_sendCountdown = SEND_INTERVAL_SECONDS_PER_NEIGHBOUR / _neighbourCount;
		}
		LOGMeshTopologyVerbose("sendCountdown=%u", _sendCountdown);
	}

	if (_sendNoopCountdown != 0) {
		_sendNoopCountdown--;
	}
	if (_sendNoopCountdown == 0) {
		sendNoop();
		_sendNoopCountdown = _fastIntervalCountdown ? SEND_NOOP_INTERVAL_SECONDS_FAST : SEND_NOOP_INTERVAL_SECONDS;
		LOGMeshTopologyVerbose("sendNoopCountdown=%u", _sendNoopCountdown);
	}

	if (_fastIntervalCountdown != 0) {
		_fastIntervalCountdown--;
	}
}

void MeshTopology::print() {
	for (uint8_t i = 0; i < _neighbourCount; ++i) {
		LOGMeshTopologyVerbose("index=%u id=%u rssi=[%i, %i, %i] secondsAgo=%u",
				i,
				_neighbours[i].id,
				_neighbours[i].rssiChannel37,
				_neighbours[i].rssiChannel38,
				_neighbours[i].rssiChannel39,
				_neighbours[i].lastSeenSecondsAgo);
	}
}

void MeshTopology::handleEvent(event_t &evt) {
	switch (evt.type) {
		case CS_TYPE::EVT_RECV_MESH_MSG: {
			auto packet = CS_TYPE_CAST(EVT_RECV_MESH_MSG, evt.data);
			onMeshMsg(*packet, evt.result);
			break;
		}
		case CS_TYPE::EVT_TICK: {
			auto packet = CS_TYPE_CAST(EVT_TICK, evt.data);
			if (*packet % (1000 / TICK_INTERVAL_MS) == 0) {
				onTickSecond();
			}
			break;
		}
		case CS_TYPE::CMD_MESH_TOPO_GET_MAC: {
			auto packet = CS_TYPE_CAST(CMD_MESH_TOPO_GET_MAC, evt.data);
			evt.result.returnCode = getMacAddress(*packet);
			break;
		}
		case CS_TYPE::CMD_MESH_TOPO_RESET: {
			reset();
			evt.result.returnCode = ERR_SUCCESS;
			break;
		}
		case CS_TYPE::CMD_MESH_TOPO_GET_RSSI: {
			auto packet = CS_TYPE_CAST(CMD_MESH_TOPO_GET_RSSI, evt.data);
			getRssi(*packet, evt.result);
			break;
		}
		default:
			break;
	}
}
