# We add the source files explicitly. This is recommended in the cmake system and will also force us all the time to
# consider the size of the final binary. Do not include things, if not necessary!

if(NOT BUILD_MESHING)
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/timer/app_timer.c")
endif()

if(DEVICE STREQUAL "nRF52832_xxAA")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52.S")
	set_property(SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52.S" PROPERTY LANGUAGE C)
elseif(DEVICE STREQUAL "nRF52833")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52833.S")
	set_property(SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52833.S" PROPERTY LANGUAGE C)
elseif(DEVICE STREQUAL "nRF52840_xxAA")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52840.S")
	set_property(SOURCE "${NRF5_DIR}/modules/nrfx/mdk/gcc_startup_nrf52840.S" PROPERTY LANGUAGE C)
else()
	message(FATAL_ERROR "Unkown device: ${DEVICE}")
endif()

# The following files are only added for the logging module by Nordic. It might be good to remove these files to
# save space in production. It should then be enclosed within a macro.
# Those files are: nrf_strerror.c

list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/ble/ble_db_discovery/ble_db_discovery.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/ble/common/ble_advdata.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/ble/common/ble_srv_common.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/atomic/nrf_atomic.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/atomic_fifo/nrf_atfifo.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/crc16/crc16.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/crc32/crc32.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/experimental_section_vars/nrf_section_iter.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/fds/fds.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/fstorage/nrf_fstorage.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/fstorage/nrf_fstorage_sd.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/hardfault/hardfault_implementation.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/hardfault/nrf52/handler/hardfault_handler_gcc.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/scheduler/app_scheduler.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/strerror/nrf_strerror.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/util/app_error.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/util/app_error_handler_gcc.c")
#	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/util/app_error_weak.c")


#	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/util/app_util_platform.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/util/nrf_assert.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/softdevice/common/nrf_sdh.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/softdevice/common/nrf_sdh_ble.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/softdevice/common/nrf_sdh_soc.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/nrfx_comp.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/nrfx_wdt.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/prs/nrfx_prs.c")
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/hal/nrf_nvmc.c")
# should be our own code, but SystemInit here contains a lot of PANs we don't have to solve subsequently...
list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/mdk/system_nrf52.c")

if(NORDIC_SDK_VERSION GREATER 15)
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/ble/nrf_ble_gq/nrf_ble_gq.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/queue/nrf_queue.c")
endif()

if(CS_SERIAL_NRF_LOG_ENABLED)
	message(STATUS "SERIAL from NORDIC enabled")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/balloc/nrf_balloc.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_backend_serial.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_default_backends.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_frontend.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_str_formatter.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/memobj/nrf_memobj.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/ringbuf/nrf_ringbuf.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/external/fprintf/nrf_fprintf.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/external/fprintf/nrf_fprintf_format.c")
	if(CS_SERIAL_NRF_LOG_ENABLED STREQUAL 1)
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_backend_rtt.c")
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/external/segger_rtt/SEGGER_RTT.c")
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/external/segger_rtt/SEGGER_RTT_printf.c")
	endif()
	if(CS_SERIAL_NRF_LOG_ENABLED STREQUAL 2)
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/log/src/nrf_log_backend_uart.c")
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/integration/nrfx/legacy/nrf_drv_uart.c")
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/nrfx_uart.c")
	endif()
else()
	if(NORDIC_SDK_VERSION GREATER 15)
		# Required now for gatt queue
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/memobj/nrf_memobj.c")
		list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/balloc/nrf_balloc.c")
	endif()
	message(STATUS "SERIAL from NORDIC disabled")
endif()

if(BUILD_MICROAPP_SUPPORT)
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/fstorage/nrf_fstorage.c")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/components/libraries/fstorage/nrf_fstorage_sd.c")
else()
	message(STATUS "Microapp support disabled, no fstorage module required")
endif()

if(BUILD_TWI)
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/nrfx_twi.c")
else()
	message(STATUS "Module for twi disabled")
endif()

if(BUILD_GPIOTE)
	message(STATUS "Add Nordic C files for nrfx gpiote driver")
	list(APPEND NORDIC_SOURCE "${NRF5_DIR}/modules/nrfx/drivers/src/nrfx_gpiote.c")
else()
	message(STATUS "Module for gpio tasks and events disabled")
endif()

