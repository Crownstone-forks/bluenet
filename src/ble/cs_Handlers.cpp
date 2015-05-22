/**
 * Author: Christopher Mason
 * Author: Anne van Rossum
 * License: TODO
 */

#include "ble/cs_Handlers.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <pstorage.h>
#include <third/protocol/rbc_mesh.h>

//#include <drivers/cs_Serial.h>

/**Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * This function is called from the scheduler in the main loop after a BLE stack
 *   event has been received.
 *
 * @p_ble_evt   Bluetooth stack event.
 */
void sys_evt_dispatch(uint32_t sys_evt) {

	// NOTE: logging here may mess up the program
//	LOGi("Sys evt dispatch: %d", sys_evt);

    pstorage_sys_event_handler(sys_evt);

#if CHAR_MESHING==1
    rbc_mesh_sys_evt_handler(sys_evt);
#endif

}

//// called by softdevice handler on a ble event
//void ble_evt_handler(ble_evt_t* p_ble_evt) {
//	Nrf51822BluetoothStack::getInstance().on_ble_evt(p_ble_evt);
//}


#ifdef __cplusplus
}
#endif

