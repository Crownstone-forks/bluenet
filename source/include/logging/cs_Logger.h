/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Dec 9, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

/**
 * Logger class.
 *
 * Generally, you want to use: LOGnone, LOGv, LOGd, LOGi, LOGw, LOGe, or LOGf.
 * These can be used like printf(), except that they will always add a newline.
 *
 * To log an array of data, use one of:
 * _logArray(level, addNewLine, pointer, size);
 * _logArray(level, addNewLine, pointer, size, elementFmt);
 * _logArray(level, addNewLine, pointer, size, startFmt, endFmt);
 * _logArray(level, addNewLine, pointer, size, startFmt, endFmt, elementFmt);
 * _logArray(level, addNewLine, pointer, size, startFmt, endFmt, elementFmt, seperationFmt);
 * _logArray(level, addNewLine, pointer, size, startFmt, endFmt, elementFmt, seperationFmt, reverse);
 * With:
 * - level:         uint8_t    The log level, for example SERIAL_INFO.
 * - addNewLine     bool       Whether to add a new line after this log.
 * - pointer        T*         Pointer to the array.
 * - size           size_t     Size of the array.
 * - startFmt       char*      Optional string to start the log. Default: "[".
 * - endFmt         char*      Optional string to end the log.   Default: "]".
 * - elementFmt     char*      Optional string to format an element value. Default depends on the element type. Example:
 * "%u" for an unsigned integer.
 * - seperationFmt  char*      Optional string to separate element values. Default: ", ".
 * - reverse        bool       Whether to print the array from last to first element.
 *
 * The log functions have to be macros, so they can add the filename and line number to the logs.
 *
 * With binary logging, the arguments are not first converted into plain text before sending them over the UART.
 * This is why templated and specialized functions are required for the implementation.
 * This saves a lot of bytes to be sent, and saves binary size, as the strings do not end up in the firmware.
 *
 * Since the log functions have an unknown amount of arguments, these macros require a variadic macro as well.
 * See http://www.wikiwand.com/en/Variadic_macro.
 * The two ## are e.g. a gcc specific extension that removes the , so that the ... arguments can also be left out.
 */

#ifdef HOST_TARGET
#include <stdio.h>
#endif

#include <cfg/cs_Strings.h>     // Should actually be included by the files that use these.
#include <drivers/cs_Serial.h>  // For SERIAL_VERBOSITY.
#include <protocol/cs_UartMsgTypes.h>

#include <cstdint>

// Deprecated, use LOGvv instead. To disable particular logs, but without commenting it.
#define LOGnone(fmt, ...)

#if !defined HOST_TARGET && (CS_SERIAL_NRF_LOG_ENABLED > 0)
	#define LOG_FLUSH NRF_LOG_FLUSH
#else
	#define LOG_FLUSH()
#endif

#if !defined HOST_TARGET && (CS_SERIAL_NRF_LOG_ENABLED > 0)

	#include<logging/impl/cs_LogNrf.h>

#else

	#if SERIAL_VERBOSITY > SERIAL_BYTE_PROTOCOL_ONLY
		#include <logging/impl/cs_LogCs.h>
	#else
		#include <logging/impl/cs_LogNone.h>
	#endif

	#include <logging/impl/cs_LogUtils.h>

#endif

// undefine macros based on SERIAL_VERBOSITY

#if SERIAL_VERBOSITY < SERIAL_VERY_VERBOSE
#undef LOGvv
#define LOGvv(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_VERBOSE
#undef LOGv
#define LOGv(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_DEBUG
#undef LOGd
#define LOGd(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_INFO
#undef LOGi
#define LOGi(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_WARN
#undef LOGw
#define LOGw(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_ERROR
#undef LOGe
#define LOGe(fmt, ...)
#endif

#if SERIAL_VERBOSITY < SERIAL_FATAL
#undef LOGf
#define LOGf(fmt, ...)
#endif
