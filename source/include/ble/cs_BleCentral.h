/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Mar 26, 2021
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <ble/cs_Nordic.h>
#include <ble/cs_UUID.h>
#include <common/cs_Types.h>
#include <events/cs_Event.h>
#include <structs/cs_PacketsInternal.h>

class BleCentral {
public:
	static BleCentral& getInstance() {
		static BleCentral instance;
		return instance;
	}

	void init();

	/**
	 * Connect to a device.
	 *
	 * @return ERR_BUSY                When already connected or another operation is in progress (discovery, read, write, connect, disconnect).
	 * @return ERR_WAIT_FOR_SUCCESS    When the connection will be attempted. Wait for the connect result event.
	 */
	cs_ret_code_t connect(const device_address_t& address, uint16_t timeoutMs = 3000);

	/**
	 * Terminate current connection.
	 *
	 * @return ERR_SUCCESS             When already disconnected.
	 * @return ERR_WAIT_FOR_SUCCESS    When disconnecting. Wait for the disconnect result event.
	 */
	cs_ret_code_t disconnect();

	/**
	 * Check whether this crownstone is connected as central.
	 */
	bool isConnected();

	/**
	 * Check whether an operation is in progress.
	 */
	bool isBusy();

	/**
	 * Discover services.
	 *
	 * Unfortunately you cannot simply discover all services, you will need to tell in advance which services you are looking for.
	 *
	 * For each discovered service, an event is dispatched.
	 *
	 * @param[in] uuids                Array of UUIDs that will be attempted to discover.
	 * @param[in] uuidCount            Number of UUIDs in the array.
	 *
	 * @return ERR_WAIT_FOR_SUCCESS    When the discovery is started. Wait for the discovery result event.
	 */
	cs_ret_code_t discoverServices(const UUID* uuids, uint8_t uuidCount);

	/**
	 * Write data to a characteristic.
	 *
	 * @param[in] handle               The characteristic handle to write to. The handle was received during discovery.
	 * @param[in] data                 Pointer to data, which will be copied.
	 * @param[in] len                  Length of the data to write.
	 *
	 * @return ERR_BUFFER_TOO_SMALL    The data size is too large.
	 * @return ERR_BUSY                An operation is in progress (discovery, read, write, connect, disconnect).
	 * @return ERR_WAIT_FOR_SUCCESS    When the write is started. Wait for the write result event.
	 */
	cs_ret_code_t write(uint16_t handle, const uint8_t* data, uint16_t len);

	/**
	 * Read data from a characteristic.
	 *
	 * @param[in] handle               The characteristic handle to read from. The handle was received during discovery.
	 *
	 * @return ERR_BUSY                An operation is in progress (discovery, read, write, connect, disconnect).
	 * @return ERR_WAIT_FOR_SUCCESS    When the read is started. Wait for the read result event.
	 *                                 This event will have a pointer to the data, which will be valid only during this event.
	 */
	cs_ret_code_t read(uint16_t handle);

private:
	enum class Operation: uint8_t {
		NONE,
		CONNECT,
		DISCONNECT,
		DISCOVERY,
		READ,
		WRITE
	};

	BleCentral();
	BleCentral(BleCentral const&);
	void operator=(BleCentral const &);

	/**
	 * Buffer used for reading and writing.
	 */
	uint8_t _buf[200];

	/**
	 * How much data is actually in the buffer.
	 */
	uint16_t _bufDataSize = 0;

	uint16_t _connectionHandle = BLE_CONN_HANDLE_INVALID;

	ble_db_discovery_t _discoveryModule;

	/**
	 * Current MTU.
	 */
	uint16_t _writeMtu = BLE_GATT_ATT_MTU_DEFAULT - 3;

	/**
	 * Operation in progress.
	 */
	Operation _currentOperation = Operation::NONE;

	/**
	 * Writes the next chunk of a long write.
	 */
	cs_ret_code_t nextWrite(uint16_t handle, uint16_t offset);

	/**
	 * Finalize an operation.
	 */
	void finalizeOperation(Operation operation, cs_ret_code_t retCode);
	void finalizeOperation(Operation operation, uint8_t* data, uint8_t dataSize);
	void sendOperationResult(event_t& event);

	/**
	 * Event handlers.
	 */
	void onGapEvent(uint16_t evtId, const ble_gap_evt_t& event);
	void onGattCentralEvent(uint16_t evtId, const ble_gattc_evt_t& event);

	void onConnect(uint16_t connectionHandle, const ble_gap_evt_connected_t& event);
	void onDisconnect(const ble_gap_evt_disconnected_t& event);
	void onGapTimeout(const ble_gap_evt_timeout_t& event);

	void onMtu(uint16_t gattStatus, const ble_gattc_evt_exchange_mtu_rsp_t& event);
	void onRead(uint16_t gattStatus, const ble_gattc_evt_read_rsp_t& event);
	void onWrite(uint16_t gattStatus, const ble_gattc_evt_write_rsp_t& event);


public:
	/**
	 * Internal usage: to be called on BLE events.
	 */
	void onBleEvent(const ble_evt_t* event);

	/**
	 * Internal usage.
	 */
	void onDiscoveryEvent(ble_db_discovery_evt_t* event);
};


