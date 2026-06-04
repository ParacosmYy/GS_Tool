/**
 * @file SerialDetectorChip.cpp
 * @brief USB芯片识别与厂商数据库 - 从SerialDetector拆分的芯片/驱动识别逻辑
 *
 * 功能:
 *   1. 已知USB转串口芯片厂商数据库(kKnownVendors)
 *   2. 根据VID查找厂商信息(lookupVendor)
 *   3. 根据VID/PID精确识别芯片型号(identifyChip)
 *   4. 根据设备描述/制造商识别驱动类型(identifyDriverType)
 */

#include "serial/detector/SerialDetector.h"

// ---- 已知USB转串口芯片数据库 ----

/**
 * @brief 已知USB转串口芯片厂商数据库
 *
 * 涵盖市面上绝大多数USB转串口芯片:
 * - Winchiphead (CH340/CH341): 最常见的国产芯片
 * - Silicon Labs (CP210x): 工业级常用
 * - FTDI (FT232/FT2232): 高性能工业级
 * - Prolific (PL2303): 早期通用芯片
 * - Microchip (MCP2200): 兼容性好的方案
 * - QinHeng (CH910/CH914): 新一代国产芯片
 * - STMicroelectronics (ST-LINK/V2): STM32调试器虚拟串口
 * - Espressif (ESP32): ESP32内置USB CDC
 */
const QVector<UsbVendorEntry> SerialDetector::kKnownVendors = {
    { 0x1A86, "Winchiphead",  "CH340"   },
    { 0x1A86, "Winchiphead",  "CH341"   },
    { 0x10C4, "Silicon Labs", "CP2102"  },
    { 0x10C4, "Silicon Labs", "CP2104"  },
    { 0x10C4, "Silicon Labs", "CP2108"  },
    { 0x10C4, "Silicon Labs", "CP2109"  },
    { 0x0403, "FTDI",         "FT232"   },
    { 0x0403, "FTDI",         "FT2232"  },
    { 0x0403, "FTDI",         "FT4232"  },
    { 0x0403, "FTDI",         "FT230X"  },
    { 0x067B, "Prolific",     "PL2303"  },
    { 0x04D8, "Microchip",    "MCP2200" },
    { 0x4348, "QinHeng",      "CH910"   },
    { 0x4348, "QinHeng",      "CH914"   },
    { 0x0483, "STMicroelectronics", "ST-LINK" },
    { 0x303A, "Espressif",    "ESP32"   },
    { 0x2E8A, "Raspberry Pi", "RP2040"  },
    { 0x16C0, "VOTI",         "Teensy"  },
    { 0x2341, "Arduino",      "UNO"     },
    { 0x2341, "Arduino",      "Mega"    },
    { 0x2886, "Seeed",        "XIAO"    },
};

// ---- 芯片识别 ----

/**
 * @brief 根据VID查找已知芯片厂商信息 @param vid USB厂商ID @return 厂商信息
 *
 * 遍历已知芯片数据库，匹配VID返回对应的厂商名称和常见芯片型号。
 * 多个条目匹配同一VID时返回第一个(最常见芯片)。
 */
UsbVendorEntry SerialDetector::lookupVendor(quint16 vid)
{
    for (const auto &entry : kKnownVendors) {
        if (entry.vid == vid) return entry;
    }
    return { vid, QString(), QString() };
}

/**
 * @brief 根据VID/PID组合识别芯片型号 @param vid USB厂商ID @param pid USB产品ID @return 芯片型号
 *
 * 精确匹配常见芯片的VID/PID组合:
 * - CH340G: VID=1A86, PID=7523
 * - CH341:  VID=1A86, PID=5523
 * - CP2102: VID=10C4, PID=EA60
 * - CP2104: VID=10C4, PID=EA70
 * - FT232R: VID=0403, PID=6001
 * - FT2232: VID=0403, PID=6010
 * - PL2303: VID=067B, PID=2303
 * - MCP2200: VID=04D8, PID=00DF
 */
QString SerialDetector::identifyChip(quint16 vid, quint16 pid)
{
    // 精确VID/PID匹配表
    static const QHash<quint32, QString> chipMap = {
        // CH340/CH341系列 (Winchiphead)
        { (0x1A86u << 16) | 0x7523u, "CH340G"  },
        { (0x1A86u << 16) | 0x5523u, "CH341"   },
        { (0x1A86u << 16) | 0x1A00u, "CH9102F" },
        // Silicon Labs CP210x系列
        { (0x10C4u << 16) | 0xEA60u, "CP2102"  },
        { (0x10C4u << 16) | 0xEA70u, "CP2104"  },
        { (0x10C4u << 16) | 0xEA71u, "CP2109"  },
        { (0x10C4u << 16) | 0x80CAu, "CP2108"  },
        { (0x10C4u << 16) | 0x8A2Au, "CP2102N" },
        // FTDI系列
        { (0x0403u << 16) | 0x6001u, "FT232R"  },
        { (0x0403u << 16) | 0x6010u, "FT2232H" },
        { (0x0403u << 16) | 0x6011u, "FT4232H" },
        { (0x0403u << 16) | 0x6014u, "FT232H"  },
        { (0x0403u << 16) | 0x6015u, "FT230X"  },
        // Prolific系列
        { (0x067Bu << 16) | 0x2303u, "PL2303H" },
        { (0x067Bu << 16) | 0x23A3u, "PL2303TA"},
        { (0x067Bu << 16) | 0x23B3u, "PL2303TB"},
        // Microchip
        { (0x04D8u << 16) | 0x00DFu, "MCP2200" },
        // ST-LINK
        { (0x0483u << 16) | 0x374Bu, "ST-LINK/V2" },
        { (0x0483u << 16) | 0x3748u, "ST-LINK/V2-1"},
        { (0x0483u << 16) | 0x374Fu, "ST-LINK/V3" },
        // Espressif
        { (0x303Au << 16) | 0x1001u, "ESP32-S2" },
        { (0x303Au << 16) | 0x0002u, "ESP32-S3" },
        // Raspberry Pi
        { (0x2E8Au << 16) | 0x000Au, "RP2040"  },
        // Arduino
        { (0x2341u << 16) | 0x0043u, "UNO R3"  },
        { (0x2341u << 16) | 0x0010u, "Mega2560"},
        { (0x2341u << 16) | 0x804Eu, "Nano 33" },
    };
    quint32 key = (static_cast<quint32>(vid) << 16) | pid;
    return chipMap.value(key, "Unknown");
}

/**
 * @brief 识别驱动类型(CH340/CP2102/FT232/PL2303等)
 *
 * 通过设备描述和制造商字段中的关键词匹配已知驱动芯片。
 * @param description 设备描述 @param manufacturer 制造商 @return 驱动类型字符串
 */
QString SerialDetector::identifyDriverType(const QString &description, const QString &manufacturer)
{
    // 合并描述和制造商进行关键词匹配
    QString combined = (description + " " + manufacturer).toUpper();

    if (combined.contains("CH340") || combined.contains("CH341"))
        return "CH340";
    if (combined.contains("CH910") || combined.contains("CH914"))
        return "CH910";
    if (combined.contains("CP210"))
        return "CP2102";
    if (combined.contains("FT232") || combined.contains("FT2232") || combined.contains("FTDI"))
        return "FT232";
    if (combined.contains("PL2303") || combined.contains("PROLIFIC"))
        return "PL2303";
    if (combined.contains("MCP2200") || combined.contains("MICROCHIP"))
        return "MCP2200";
    if (combined.contains("ST-LINK") || combined.contains("STLINK"))
        return "ST-LINK";
    if (combined.contains("ESP32") || combined.contains("ESPRESSIF"))
        return "ESP32";
    if (combined.contains("RP2040") || combined.contains("RASPBERRY"))
        return "RP2040";

    return "Unknown";
}
