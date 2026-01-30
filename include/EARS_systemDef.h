/**
 * @file EARS_systemDef.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Project System Header File.
 * @version 0.1.0
 * @date 20260121
 *
 *
 */
#pragma once
#ifndef __EARS_SYSTEM_DEF_H__
#define __EARS_SYSTEM_DEF_H__

/**********************************************************************
 * System Definitions
 **********************************************************************/

#define EARS_SYSTEM_NAME "EARS"
#define EARS_DESCRIPTION "Equipment & Ammunition Reporting System"
#define EARS_STATUS "Development"
#define EARS_COPYRIGHT "Copyright JTB 2026 All Rights Reserved"

/**********************************************************************
 * Debug Definitions
 **********************************************************************/
#define EARS_DEBUG_BAUD_RATE 115200
#if EARS_DEBUG == 1
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

/**********************************************************************
 * NVS Definitions
 **********************************************************************/
#define EARS_NAMESPACE "EARS"          // NVS Namespace
#define EARS_VERSION_CODE "EARS_VC"    // Version Code (2 Hexadecimal Characters (0-9, A-F to Represent Version of NVS Data)
#define EARS_ZAPCODE "EARS_ZC"         // 6 Character Code to Marry the Device to a User. It is 2 Uppercase Letters Followed by 4 Digits.
#define EARS_PASSWORD_HASH "EARS_PW"   // A CRC32 hash of the Password created by the User.
#define EARS_BACKLIGHT_VALUE "EARS_BL" // Backlight Brightness Value (0-100) as 1 Byte
#define EARS_CRC32 "EARS_32"           // CRC32 of all NVS Data to Ensure Integrity

#endif // __EARS_SYSTEM_DEF_H__