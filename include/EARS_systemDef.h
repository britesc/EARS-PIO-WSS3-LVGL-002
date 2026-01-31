/**
 * @file EARS_systemDef.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Project System Header File.
 * @version 0.2.0
 * @date 20260130
 *
 * @details
 * System-wide definitions. Updated to use constexpr for type safety
 * while maintaining backward compatibility with existing code.
 */
#pragma once
#ifndef __EARS_SYSTEM_DEF_H__
#define __EARS_SYSTEM_DEF_H__

/**********************************************************************
 * System Definitions (constexpr for type safety)
 **********************************************************************/
namespace EARS_Internal
{
    constexpr const char *SYSTEM_NAME_STR = "EARS";
    constexpr const char *DESCRIPTION_STR = "Equipment & Ammunition Reporting System";
    constexpr const char *STATUS_STR = "Development";
    constexpr const char *COPYRIGHT_STR = "Copyright JTB 2026 All Rights Reserved";
}

// Legacy compatibility - existing code uses these
#define EARS_SYSTEM_NAME EARS_Internal::SYSTEM_NAME_STR
#define EARS_DESCRIPTION EARS_Internal::DESCRIPTION_STR
#define EARS_STATUS EARS_Internal::STATUS_STR
#define EARS_COPYRIGHT EARS_Internal::COPYRIGHT_STR

/**********************************************************************
 * Debug Definitions (preprocessor required)
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
 * NVS Definitions (constexpr with backward compatibility)
 **********************************************************************/
namespace EARS_Internal
{
    namespace NVS
    {
        constexpr const char *NAMESPACE_STR = "EARS";
        constexpr const char *VERSION_CODE_STR = "EARS_VC";
        constexpr const char *ZAPCODE_STR = "EARS_ZC";
        constexpr const char *PASSWORD_HASH_STR = "EARS_PW";
        constexpr const char *BACKLIGHT_VALUE_STR = "EARS_BL";
        constexpr const char *CRC32_STR = "EARS_32";
    }
}

// Legacy compatibility - existing code uses these
#define EARS_NAMESPACE EARS_Internal::NVS::NAMESPACE_STR
#define EARS_VERSION_CODE EARS_Internal::NVS::VERSION_CODE_STR
#define EARS_ZAPCODE EARS_Internal::NVS::ZAPCODE_STR
#define EARS_PASSWORD_HASH EARS_Internal::NVS::PASSWORD_HASH_STR
#define EARS_BACKLIGHT_VALUE EARS_Internal::NVS::BACKLIGHT_VALUE_STR
#define EARS_CRC32 EARS_Internal::NVS::CRC32_STR

#endif // __EARS_SYSTEM_DEF_H__