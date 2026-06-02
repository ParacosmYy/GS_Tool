/**
 * @file CRC.h
 * @brief CRC校验计算工具集 - 提供CRC8/CRC16-CCITT/XMODEM/CRC16-Modbus/CRC32/Checksum
 *
 * 全部为静态内联函数，无需实例化。用于OTA协议帧校验、Intel HEX校验和验证等。
 * CRC8(0x07) / CRC16-CCITT(0x1021) / CRC16-XMODEM(CCITT别名) /
 * CRC16-Modbus(0x8005/0xA001) / CRC32(0xEDB88320) / Checksum(算术和)
 */
#ifndef CRC_H
#define CRC_H

#include <cstdint>
#include <QByteArray>

/// CRC校验计算工具 - 用于OTA协议和帧校验
/// 全部是静态方法，不需要实例化
namespace CRC {

// ---- CRC8 ----

/**
 * @brief CRC8计算 (多项式 0x07, 初始值 0x00)
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC8校验值
 */
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

/// @brief QByteArray重载版本
inline uint8_t crc8(const QByteArray& ba)
{
    return crc8(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// ---- CRC16-CCITT (XMODEM标准) ----

/** @brief CRC16-CCITT计算 (多项式 0x1021, 初始值 0x0000, XMODEM标准)
 *  @param data 数据指针 @param length 数据长度 @return CRC16校验值 */
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

/// @brief QByteArray重载版本
inline uint16_t crc16Ccitt(const QByteArray& ba)
{
    return crc16Ccitt(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// ---- CRC16-XMODEM (语义别名，与crc16Ccitt完全相同) ----

/** @brief CRC16-XMODEM计算(XMODEM协议规范名称，语义别名)
 *  @param data 数据指针 @param length 数据长度 @return CRC16校验值 */
inline uint16_t crc16Xmodem(const uint8_t* data, int length)
{
    return crc16Ccitt(data, length);
}

/// @brief QByteArray重载版本
inline uint16_t crc16Xmodem(const QByteArray& ba)
{
    return crc16Xmodem(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// ---- CRC16-Modbus ----

/** @brief CRC16-Modbus计算 (多项式 0x8005/反转0xA001, 初始值 0xFFFF, LSB-first)
 *  @param data 数据指针 @param length 数据长度 @return CRC16校验值 */
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

/// @brief QByteArray重载版本
inline uint16_t crc16Modbus(const QByteArray& ba)
{
    return crc16Modbus(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// ---- CRC32 ----

/** @brief CRC32计算 (多项式 0xEDB88320, 初始值 0xFFFFFFFF, 用于ZMODEM/ZIP)
 *  @param data 数据指针 @param length 数据长度 @return CRC32校验值 */
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

/// @brief QByteArray重载版本
inline uint32_t crc32(const QByteArray& ba)
{
    return crc32(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

// ---- 算术校验和 (XMODEM-Checksum模式) ----

/** @brief 算术校验和: 所有字节累加取低8位
 *  @param data 数据指针 @param length 数据长度 @return 校验和 */
inline uint8_t checksum(const uint8_t* data, int length)
{
    uint8_t sum = 0;
    for (int i = 0; i < length; ++i) {
        sum += data[i];
    }
    return sum;
}

/// @brief QByteArray重载版本
inline uint8_t checksum(const QByteArray& ba)
{
    return checksum(reinterpret_cast<const uint8_t*>(ba.constData()), ba.size());
}

} // namespace CRC

#endif // CRC_H
