/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Nov 1, 2021
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <events/cs_EventListener.h>
#include <dfu/cs_FirmwareSections.h>
#include <util/cs_Coroutine.h>

/**
 * This class reads pieces of the firmware that is currently running on this device
 * in order to enabling dfu-ing itself onto another (outdated) crownstone.
 */
class FirmwareReader : public EventListener {
public:

	/**
	 * current offset in section (for printing purposes)
	 */
	uint16_t dataoffSet = 0;

	FirmwareReader();

	/**
	 * reads flash using fstorage
	 */
	void read(uint16_t startIndex, uint16_t size, void* data_out);


	cs_ret_code_t init();

protected:
private:
public:
	virtual void handleEvent(event_t& evt) override;

	/**
	 * routine for printing small chunks of the configured flash section.
	 */
	Coroutine firmwarePrinter;
	uint32_t printRoutine();
};
