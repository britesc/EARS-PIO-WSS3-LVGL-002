/**
 * @file EARS_versionDef.h
 * @author Julian (51fiftyone51fiftyone_at_gmail.com)
 * @brief Version encoding/decoding macros for EARS Project
 * @version 0.7.0
 * @date 20260122
 * 
 * Converts version string components into a single integer for easy comparison
 * Format: MMMMMMPPPPPPBBBBBB (Major.Minor.Patch as 9 digits)
 * Example: 4.0.67 becomes 004000067
 */
#pragma once
#ifndef __EARS_VERSION_DEF_H__
#define __EARS_VERSION_DEF_H__

#include <stdint.h>

#define EARS_APP_VERSION_MAJOR "4"
#define EARS_APP_VERSION_MINOR "2"
#define EARS_APP_VERSION_PATCH "120"

#define EARS_APP_BUILD_TIMESTAMP 20260131113315

/**
 * @brief Convert string to integer at compile time
 * Helper macro for VERS_ENCODE
 */
#define VERS_ATOI_HELPER(x) ((x[0] >= '0' && x[0] <= '9') ? \
    ((x[0] - '0') * 100 + \
     (x[1] >= '0' && x[1] <= '9' ? (x[1] - '0') * 10 : 0) + \
     (x[2] >= '0' && x[2] <= '9' ? (x[2] - '0') : 0)) : 0)

/**
 * @brief Encode version components into single integer
 * @param maj Major version string (e.g., "4")
 * @param min Minor version string (e.g., "0")
 * @param pat Patch version string (e.g., "67")
 * @return Integer version code (e.g., 4000067)
 * 
 * Usage: uint32_t version = VERS_ENCODE(EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR, EARS_APP_VERSION_PATCH);
 */
#define VERS_ENCODE(maj, min, pat) \
    ((uint32_t)(VERS_ATOI_HELPER(maj) * 1000000UL + \
                VERS_ATOI_HELPER(min) * 1000UL + \
                VERS_ATOI_HELPER(pat)))

/**
 * @brief Extract major version from encoded integer
 * @param v Encoded version integer
 * @return Major version number
 */
#define VERS_GET_MAJOR(v) ((v) / 1000000UL)

/**
 * @brief Extract minor version from encoded integer
 * @param v Encoded version integer
 * @return Minor version number
 */
#define VERS_GET_MINOR(v) (((v) / 1000UL) % 1000UL)

/**
 * @brief Extract patch version from encoded integer
 * @param v Encoded version integer
 * @return Patch version number
 */
#define VERS_GET_PATCH(v) ((v) % 1000UL)

/**
 * @brief Format version integer into string buffer
 * @param v Encoded version integer
 * @param buf Character buffer (must be at least 12 bytes)
 * 
 * Usage: 
 * char version_str[12];
 * VERS_FORMAT(version_code, version_str);
 * printf("Version: %s\n", version_str);
 */
#define VERS_FORMAT(v, buf) \
    snprintf(buf, 12, "%d.%d.%d", \
             (int)VERS_GET_MAJOR(v), \
             (int)VERS_GET_MINOR(v), \
             (int)VERS_GET_PATCH(v))

             // Macro to get readable date/time from EARS_APP_BUILD_TIMESTAMP
             #define EARS_APP_BUILD_YEAR    ((EARS_APP_BUILD_TIMESTAMP / 10000000000ULL) % 10000)
             #define EARS_APP_BUILD_MONTH   ((EARS_APP_BUILD_TIMESTAMP / 100000000ULL) % 100)
             #define EARS_APP_BUILD_DAY     ((EARS_APP_BUILD_TIMESTAMP / 1000000ULL) % 100)
             #define EARS_APP_BUILD_HOUR    ((EARS_APP_BUILD_TIMESTAMP / 10000ULL) % 100)
             #define EARS_APP_BUILD_MINUTE  ((EARS_APP_BUILD_TIMESTAMP / 100ULL) % 100)
             #define EARS_APP_BUILD_SECOND  (EARS_APP_BUILD_TIMESTAMP % 100ULL)

#endif // __EARS_VERSION_DEF_H__