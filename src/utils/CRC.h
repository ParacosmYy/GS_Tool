#ifndef CRC_H
#define CRC_H

#include <cstdint>
#include <QByteArray>

// CRC校验计算工具 - 用于OTA协议和帧校验
// 全部是静态方法，不需要实例化
namespace CRC {

// CRC8计算 (多项式 0x07)
inline uint8_t crc8(const uint8_t* data, int length)
{
    uint8_t crc = 0x00;
    for (int i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

inline uint8_t crc8(const QByteArray& ba)
{
    return crc8(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// CRC16-CCITT计算 (多项式 0x1021, XMODEM标准)
inline uint16_t crc16Ccitt(const uint8_t* data, int length)
{
    uint16_t crc = 0x0000;
    for (int i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

inline uint16_t crc16Ccitt(const QByteArray& ba)
{
    return crc16Ccitt(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// CRC16-Modbus计算 (多项式 0x8005)
inline uint16_t crc16Modbus(const uint8_t* data, int length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

inline uint16_t crc16Modbus(const QByteArray& ba)
{
    return crc16Modbus(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// CRC32计算 (多项式 0xEDB88320, 用于ZMODEM等)
inline uint32_t crc32(const uint8_t* data, int length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

inline uint32_t crc32(const QByteArray& ba)
{
    return crc32(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// 算术校验和 (用于XMODEM-Checksum)
inline uint8_t checksum(const uint8_t* data, int length)
{
    uint8_t sum = 0;
    for (int i = 0; i < length; ++i) {
        sum += data[i];
    }
    return sum;
}

inline uint8_t checksum(const QByteArray& ba)
{
    return checksum(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

} // namespace CRC

#endif // CRC_H
