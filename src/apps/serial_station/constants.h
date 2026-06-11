#pragma once

#include <QString>

namespace serial_station {

namespace paths {
inline const char* const kDefaultConfigFile = "serial_station_config.json";
inline const char* const kDefaultProfileFile = "serial_station_profiles.json";
} // namespace paths

namespace window {
inline const char* const kWindowObjectName = "serialStationWindow";
inline const char* const kObjectNamePrefix = "serialStation";
inline const int kMinWindowWidth = 960;
inline const int kMinWindowHeight = 640;
} // namespace window

namespace protocol {
inline const char* const kDefaultProtocol = "ASCII_TEXT";
} // namespace protocol

} // namespace serial_station
