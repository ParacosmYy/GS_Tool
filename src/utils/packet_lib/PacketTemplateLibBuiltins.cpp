/**
 * @file PacketTemplateLibBuiltins.cpp
 * @brief 报文模板库 — 内置模板数据与 JSON 加载
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 包含 Modbus RTU / SPI / CAN / UART AT / 自定义 共 10 个内置模板，
 * 以及从 JSON 文件加载自定义模板的逻辑。
 */

#include "utils/packet_lib/PacketTemplateLib.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// ============================================================
// Modbus RTU 内置模板
// ============================================================

/**
 * @brief 创建 Modbus RTU 内置模板（FC=03/06/04）
 */
void PacketTemplateLib::createBuiltinModbusTemplates()
{
    // 读保持寄存器 (FC=03)
    PacketTemplate readHolding;
    readHolding.name = tr("读保持寄存器 (FC=03)");
    readHolding.description = tr("Modbus RTU 读取保持寄存器，功能码 0x03");
    readHolding.category = TemplateCategory::ModbusRtu;
    readHolding.frameTemplate = QByteArray::fromHex("010300000001aacha");
    readHolding.hasChecksum = true;
    readHolding.checksumOffset = 0;
    readHolding.params = {
        {tr("从站地址"), "01", 0, 1, true},
        {tr("起始地址"), "0000", 2, 2, true},
        {tr("寄存器数量"), "0001", 4, 2, true}
    };
    m_templates.append(readHolding);

    // 写单个寄存器 (FC=06)
    PacketTemplate writeSingle;
    writeSingle.name = tr("写单个寄存器 (FC=06)");
    writeSingle.description = tr("Modbus RTU 写单个保持寄存器，功能码 0x06");
    writeSingle.category = TemplateCategory::ModbusRtu;
    writeSingle.frameTemplate = QByteArray::fromHex("010600000000bcch");
    writeSingle.hasChecksum = true;
    writeSingle.checksumOffset = 0;
    writeSingle.params = {
        {tr("从站地址"), "01", 0, 1, true},
        {tr("寄存器地址"), "0000", 2, 2, true},
        {tr("写入值"), "0000", 4, 2, true}
    };
    m_templates.append(writeSingle);

    // 读输入寄存器 (FC=04)
    PacketTemplate readInput;
    readInput.name = tr("读输入寄存器 (FC=04)");
    readInput.description = tr("Modbus RTU 读取输入寄存器，功能码 0x04");
    readInput.category = TemplateCategory::ModbusRtu;
    readInput.frameTemplate = QByteArray::fromHex("010400000001b1ch");
    readInput.hasChecksum = true;
    readInput.checksumOffset = 0;
    readInput.params = {
        {tr("从站地址"), "01", 0, 1, true},
        {tr("起始地址"), "0000", 2, 2, true},
        {tr("寄存器数量"), "0001", 4, 2, true}
    };
    m_templates.append(readInput);
}

// ============================================================
// SPI 内置模板
// ============================================================

/**
 * @brief 创建 SPI 内置模板（写3字节 / 读2字节）
 */
void PacketTemplateLib::createBuiltinSpiTemplates()
{
    // SPI 写命令: [CMD][ADDR_H][ADDR_L][DATA]
    PacketTemplate spiWrite;
    spiWrite.name = tr("SPI 写命令 (3字节)");
    spiWrite.description = tr("SPI 写入: 命令 + 地址 + 数据，累加和校验");
    spiWrite.category = TemplateCategory::Spi;
    spiWrite.frameTemplate = QByteArray::fromHex("020010FFxx");
    spiWrite.hasChecksum = true;
    spiWrite.checksumOffset = 0;
    spiWrite.params = {
        {tr("命令码"), "02", 0, 1, true},
        {tr("地址"), "0010", 1, 2, true},
        {tr("数据"), "FF", 3, 1, true}
    };
    m_templates.append(spiWrite);

    // SPI 读命令: [CMD][ADDR_H][ADDR_L]
    PacketTemplate spiRead;
    spiRead.name = tr("SPI 读命令 (2字节)");
    spiRead.description = tr("SPI 读取: 命令 + 地址");
    spiRead.category = TemplateCategory::Spi;
    spiRead.frameTemplate = QByteArray::fromHex("030010");
    spiRead.hasChecksum = true;
    spiRead.checksumOffset = 0;
    spiRead.params = {
        {tr("命令码"), "03", 0, 1, true},
        {tr("地址"), "0010", 1, 2, true}
    };
    m_templates.append(spiRead);
}

// ============================================================
// CAN 内置模板
// ============================================================

/**
 * @brief 创建 CAN 内置模板（标准帧 11-bit / 扩展帧 29-bit）
 */
void PacketTemplateLib::createBuiltinCanTemplates()
{
    // CAN 标准帧 (11-bit ID): [ID_H][ID_L][DLC][D0..D7]
    PacketTemplate canStd;
    canStd.name = tr("CAN 标准帧 (11位ID)");
    canStd.description = tr("CAN 2.0A 标准帧，11位标识符 + 8字节数据");
    canStd.category = TemplateCategory::Can;
    canStd.frameTemplate = QByteArray(11, 0x00);
    canStd.hasChecksum = false;
    canStd.checksumOffset = 0;
    canStd.params = {
        {tr("CAN ID"), "0123", 0, 2, true},
        {tr("DLC"),     "08",   2, 1, true},
        {tr("数据0"),   "00",   3, 1, true},
        {tr("数据1"),   "00",   4, 1, true},
        {tr("数据2"),   "00",   5, 1, true},
        {tr("数据3"),   "00",   6, 1, true},
        {tr("数据4"),   "00",   7, 1, true},
        {tr("数据5"),   "00",   8, 1, true},
        {tr("数据6"),   "00",   9, 1, true},
        {tr("数据7"),   "00",   10, 1, true}
    };
    m_templates.append(canStd);

    // CAN 扩展帧 (29-bit ID): [ID_0..3][DLC][D0..D7]
    PacketTemplate canExt;
    canExt.name = tr("CAN 扩展帧 (29位ID)");
    canExt.description = tr("CAN 2.0B 扩展帧，29位标识符 + 8字节数据");
    canExt.category = TemplateCategory::Can;
    canExt.frameTemplate = QByteArray(13, 0x00);
    canExt.hasChecksum = false;
    canExt.checksumOffset = 0;
    canExt.params = {
        {tr("CAN ID"), "00000123", 0, 4, true},
        {tr("DLC"),     "08",       4, 1, true},
        {tr("数据0"),   "00",       5, 1, true},
        {tr("数据1"),   "00",       6, 1, true},
        {tr("数据2"),   "00",       7, 1, true},
        {tr("数据3"),   "00",       8, 1, true},
        {tr("数据4"),   "00",       9, 1, true},
        {tr("数据5"),   "00",      10, 1, true},
        {tr("数据6"),   "00",      11, 1, true},
        {tr("数据7"),   "00",      12, 1, true}
    };
    m_templates.append(canExt);
}

// ============================================================
// UART AT + 自定义内置模板
// ============================================================

/**
 * @brief 创建 UART AT 内置模板和自定义空模板
 */
void PacketTemplateLib::createBuiltinAtTemplates()
{
    // AT+RST
    PacketTemplate atRst;
    atRst.name = tr("AT+RST (复位)");
    atRst.description = tr("AT 命令: 复位模块");
    atRst.category = TemplateCategory::UartAt;
    atRst.frameTemplate = "AT+RST\r\n";
    atRst.hasChecksum = false;
    atRst.checksumOffset = 0;
    m_templates.append(atRst);

    // AT+CWLAP
    PacketTemplate atScan;
    atScan.name = tr("AT+CWLAP (WiFi扫描)");
    atScan.description = tr("AT 命令: 扫描可用 WiFi 热点");
    atScan.category = TemplateCategory::UartAt;
    atScan.frameTemplate = "AT+CWLAP\r\n";
    atScan.hasChecksum = false;
    atScan.checksumOffset = 0;
    m_templates.append(atScan);

    // 自定义空模板（8字节全零）
    PacketTemplate customEmpty;
    customEmpty.name = tr("自定义模板");
    customEmpty.description = tr("空白自定义模板，可自由编辑");
    customEmpty.category = TemplateCategory::Custom;
    customEmpty.frameTemplate = QByteArray(8, 0x00);
    customEmpty.hasChecksum = false;
    customEmpty.checksumOffset = 0;
    customEmpty.params = {
        {tr("字节0"), "00", 0, 1, true},
        {tr("字节1"), "00", 1, 1, true},
        {tr("字节2"), "00", 2, 1, true},
        {tr("字节3"), "00", 3, 1, true},
        {tr("字节4"), "00", 4, 1, true},
        {tr("字节5"), "00", 5, 1, true},
        {tr("字节6"), "00", 6, 1, true},
        {tr("字节7"), "00", 7, 1, true}
    };
    m_templates.append(customEmpty);
}

// ============================================================
// JSON 加载
// ============================================================

/**
 * @brief 从 JSON 文件加载自定义模板
 * @param filePath JSON 文件路径
 * @return true 加载成功
 *
 * JSON 格式: 数组，每个元素包含 name/description/category/frameHex/
 *           hasChecksum/checksumOffset/params 数组。
 */
bool PacketTemplateLib::loadFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_statusLabel->setText(tr("无法打开文件: %1").arg(filePath));
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray()) {
        m_statusLabel->setText(tr("JSON 格式错误: 期望数组"));
        return false;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        if (!val.isObject()) { continue; }
        QJsonObject obj = val.toObject();

        PacketTemplate tmpl;
        tmpl.name = obj["name"].toString(tr("未命名模板"));
        tmpl.description = obj["description"].toString();
        tmpl.category = static_cast<TemplateCategory>(
            obj["category"].toInt(static_cast<int>(TemplateCategory::Custom)));
        tmpl.frameTemplate = QByteArray::fromHex(obj["frameHex"].toString().toUtf8());
        tmpl.hasChecksum = obj["hasChecksum"].toBool(false);
        tmpl.checksumOffset = obj["checksumOffset"].toInt(0);

        const QJsonArray paramsArr = obj["params"].toArray();
        for (const QJsonValue &pv : paramsArr) {
            QJsonObject po = pv.toObject();
            TemplateParam p;
            p.name = po["name"].toString();
            p.value = po["value"].toString("00");
            p.byteOffset = po["byteOffset"].toInt(0);
            p.byteLength = po["byteLength"].toInt(1);
            p.isHex = po["isHex"].toBool(true);
            tmpl.params.append(p);
        }
        m_templates.append(tmpl);
    }

    m_stats.totalTemplatesLoaded = static_cast<quint64>(m_templates.size());
    populateTree();
    m_statusLabel->setText(tr("已加载 %1 个自定义模板").arg(arr.size()));
    return true;
}
