/**
 * @file SvdParser.cpp
 * @brief CMSIS SVD 文件解析器 - XML 解析实现
 *
 * 使用 QXmlStreamReader 流式解析 CMSIS SVD XML 文件，
 * 构建 Device → Peripheral → Register → Field 层级数据树。
 * 统计接口见 SvdParserStats.cpp。
 */

#include "protocol/svd/SvdParser.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

// ──────────────────────── 构造/析构 ────────────────────────

/** @brief 构造 SVD 解析器 @param parent 父对象 */
SvdParser::SvdParser(QObject* parent)
    : QObject(parent)
{
}

// ──────────────────────── 文件加载 ────────────────────────

/**
 * @brief 从文件加载 SVD XML
 * @param filePath SVD 文件路径
 * @return true=解析成功
 */
bool SvdParser::loadSvdFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = tr("无法打开 SVD 文件: %1").arg(filePath);
        ++m_stats.totalParseErrors;
        emit parseError(m_lastError);
        return false;
    }

    QXmlStreamReader xml(&file);
    const bool ok = parseDevice(xml);
    file.close();

    if (ok) {
        ++m_stats.totalFilesLoaded;
        emit parseCompleted(static_cast<int>(m_device.peripherals.size()));
    }
    return ok;
}

/**
 * @brief 从 XML 文本内容解析 SVD
 * @param xmlContent SVD XML 字符串
 * @return true=解析成功
 */
bool SvdParser::loadSvdContent(const QString& xmlContent)
{
    QXmlStreamReader xml(xmlContent);
    const bool ok = parseDevice(xml);

    if (ok) {
        emit parseCompleted(static_cast<int>(m_device.peripherals.size()));
    }
    return ok;
}

// ──────────────────────── 核心解析 ────────────────────────

/**
 * @brief 解析顶层 <device> 元素
 * @param xml XML 流读取器
 * @return true=解析成功
 */
bool SvdParser::parseDevice(QXmlStreamReader& xml)
{
    clear();

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            const QStringRef tag = xml.name();
            if (tag == QLatin1String("name")) {
                m_device.name = xml.readElementText();
            } else if (tag == QLatin1String("vendor")) {
                m_device.vendor = xml.readElementText();
            } else if (tag == QLatin1String("description")) {
                m_device.description = xml.readElementText();
            } else if (tag == QLatin1String("addressUnitBits")) {
                m_device.addressUnitBits = static_cast<quint32>(
                    parseHexValue(xml.readElementText()));
            } else if (tag == QLatin1String("width")) {
                m_device.width = static_cast<quint32>(
                    parseHexValue(xml.readElementText()));
            } else if (tag == QLatin1String("size")) {
                m_device.size = parseHexValue(xml.readElementText());
            } else if (tag == QLatin1String("access")) {
                m_device.access = xml.readElementText();
            } else if (tag == QLatin1String("resetValue")) {
                m_device.resetValue = parseHexValue(xml.readElementText());
            } else if (tag == QLatin1String("resetMask")) {
                m_device.resetMask = parseHexValue(xml.readElementText());
            } else if (tag == QLatin1String("peripherals")) {
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() &&
                        xml.name() == QLatin1String("peripherals")) {
                        break;
                    }
                    if (xml.isStartElement() &&
                        xml.name() == QLatin1String("peripheral")) {
                        parsePeripheral(xml);
                    }
                }
            }
        }

        if (xml.isEndElement() && xml.name() == QLatin1String("device")) {
            break;
        }
    }

    if (xml.hasError()) {
        m_lastError = tr("SVD XML 解析错误 (行 %1): %2")
                          .arg(xml.lineNumber()).arg(xml.errorString());
        ++m_stats.totalParseErrors;
        emit parseError(m_lastError);
        return false;
    }

    // 构建外设查找索引
    for (int i = 0; i < m_device.peripherals.size(); ++i) {
        m_peripheralIndex[m_device.peripherals[i].name] = i;
    }

    return true;
}

/**
 * @brief 解析 <peripheral> 元素
 * @param xml XML 流读取器
 */
void SvdParser::parsePeripheral(QXmlStreamReader& xml)
{
    SvdPeripheral periph;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() &&
            xml.name() == QLatin1String("peripheral")) {
            break;
        }

        if (!xml.isStartElement()) continue;

        const QStringRef tag = xml.name();

        if (tag == QLatin1String("name")) {
            periph.name = xml.readElementText();
        } else if (tag == QLatin1String("displayName")) {
            periph.displayName = xml.readElementText();
        } else if (tag == QLatin1String("description")) {
            periph.description = xml.readElementText();
        } else if (tag == QLatin1String("baseAddress")) {
            periph.baseAddress = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("size")) {
            periph.size = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("access")) {
            periph.access = xml.readElementText();
        } else if (tag == QLatin1String("groupName")) {
            periph.groupName = xml.readElementText();
        } else if (tag == QLatin1String("registers")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement() &&
                    xml.name() == QLatin1String("registers")) {
                    break;
                }
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("register")) {
                        parseRegister(xml, periph.registers);
                    } else if (xml.name() == QLatin1String("cluster")) {
                        parseCluster(xml, periph.clusters);
                    }
                }
            }
        } else if (tag == QLatin1String("cluster")) {
            parseCluster(xml, periph.clusters);
        }
    }

    ++m_stats.totalPeripherals;
    m_device.peripherals.append(periph);
}

/**
 * @brief 解析 <register> 元素
 * @param xml XML 流读取器
 * @param out 输出到寄存器列表
 */
void SvdParser::parseRegister(QXmlStreamReader& xml, QVector<SvdRegister>& out)
{
    SvdRegister reg;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() &&
            xml.name() == QLatin1String("register")) {
            break;
        }

        if (!xml.isStartElement()) continue;

        const QStringRef tag = xml.name();

        if (tag == QLatin1String("name")) {
            reg.name = xml.readElementText();
        } else if (tag == QLatin1String("displayName")) {
            reg.displayName = xml.readElementText();
        } else if (tag == QLatin1String("description")) {
            reg.description = xml.readElementText();
        } else if (tag == QLatin1String("addressOffset")) {
            reg.addressOffset = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("size")) {
            reg.size = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("access")) {
            reg.access = xml.readElementText();
        } else if (tag == QLatin1String("resetValue")) {
            reg.resetValue = parseHexValue(xml.readElementText());
        } else if (tag == QLatin1String("resetMask")) {
            reg.resetMask = parseHexValue(xml.readElementText());
        } else if (tag == QLatin1String("fields")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement() &&
                    xml.name() == QLatin1String("fields")) {
                    break;
                }
                if (xml.isStartElement() &&
                    xml.name() == QLatin1String("field")) {
                    parseField(xml, reg.fields);
                }
            }
        }
    }

    ++m_stats.totalRegisters;
    m_stats.totalFields += static_cast<quint64>(reg.fields.size());
    out.append(reg);
}

/**
 * @brief 解析 <cluster> 元素
 * @param xml XML 流读取器
 * @param out 输出到簇列表
 */
void SvdParser::parseCluster(QXmlStreamReader& xml, QVector<SvdCluster>& out)
{
    SvdCluster cluster;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() &&
            xml.name() == QLatin1String("cluster")) {
            break;
        }

        if (!xml.isStartElement()) continue;

        const QStringRef tag = xml.name();

        if (tag == QLatin1String("name")) {
            cluster.name = xml.readElementText();
        } else if (tag == QLatin1String("description")) {
            cluster.description = xml.readElementText();
        } else if (tag == QLatin1String("addressOffset")) {
            cluster.addressOffset = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("size")) {
            cluster.size = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("register")) {
            parseRegister(xml, cluster.registers);
        } else if (tag == QLatin1String("cluster")) {
            parseCluster(xml, cluster.clusters);
        }
    }

    ++m_stats.totalClusters;
    out.append(cluster);
}

/**
 * @brief 解析 <field> 元素
 * @param xml XML 流读取器
 * @param out 输出到字段列表
 */
void SvdParser::parseField(QXmlStreamReader& xml, QVector<SvdField>& out)
{
    SvdField field;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() &&
            xml.name() == QLatin1String("field")) {
            break;
        }

        if (!xml.isStartElement()) continue;

        const QStringRef tag = xml.name();

        if (tag == QLatin1String("name")) {
            field.name = xml.readElementText();
        } else if (tag == QLatin1String("description")) {
            field.description = xml.readElementText();
        } else if (tag == QLatin1String("bitOffset")) {
            field.bitOffset = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("bitWidth")) {
            field.bitWidth = static_cast<quint32>(
                parseHexValue(xml.readElementText()));
        } else if (tag == QLatin1String("access")) {
            field.access = xml.readElementText();
        } else if (tag == QLatin1String("enumeratedValues")) {
            parseEnumeratedValues(xml, field.enumeratedValues);
        }
    }

    out.append(field);
}

/**
 * @brief 解析 <enumeratedValues> 元素
 * @param xml XML 流读取器
 * @param out 输出到枚举值列表
 */
void SvdParser::parseEnumeratedValues(QXmlStreamReader& xml,
                                       QVector<SvdEnumeratedValue>& out)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() &&
            xml.name() == QLatin1String("enumeratedValues")) {
            break;
        }

        if (xml.isStartElement() &&
            xml.name() == QLatin1String("enumeratedValue")) {
            SvdEnumeratedValue ev;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement() &&
                    xml.name() == QLatin1String("enumeratedValue")) {
                    break;
                }
                if (!xml.isStartElement()) continue;
                const QStringRef tag = xml.name();
                if (tag == QLatin1String("name")) {
                    ev.name = xml.readElementText();
                } else if (tag == QLatin1String("description")) {
                    ev.description = xml.readElementText();
                } else if (tag == QLatin1String("value")) {
                    ev.value = parseHexValue(xml.readElementText());
                }
            }
            out.append(ev);
        }
    }
}

// ──────────────────────── 工具方法 ────────────────────────

/**
 * @brief 从十六进制或十进制字符串解析数值
 * @param value 字符串值 (支持 0x 前缀十六进制)
 * @return 解析结果，失败返回 0
 */
quint64 SvdParser::parseHexValue(const QString& value) const
{
    bool ok = false;
    quint64 result = 0;

    if (value.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
        result = value.toULongLong(&ok, 16);
    } else if (value.startsWith(QLatin1String("#"))) {
        result = value.mid(1).toULongLong(&ok, 16);
    } else {
        result = value.toULongLong(&ok, 10);
    }

    return ok ? result : 0;
}

// ──────────────────────── 数据查询 ────────────────────────

/** @brief 获取已解析的完整设备描述 */
SvdDevice SvdParser::device() const
{
    return m_device;
}

/** @brief 获取所有外设列表 */
QVector<SvdPeripheral> SvdParser::peripherals() const
{
    return m_device.peripherals;
}

/**
 * @brief 按名称查找外设
 * @param name 外设名称
 * @return 外设描述，未找到时 name 为空
 */
SvdPeripheral SvdParser::peripheral(const QString& name) const
{
    auto it = m_peripheralIndex.constFind(name);
    if (it != m_peripheralIndex.constEnd() &&
        *it < m_device.peripherals.size()) {
        return m_device.peripherals.at(*it);
    }
    return SvdPeripheral{};
}

/**
 * @brief 按外设名+寄存器名查找寄存器
 * @param peripheralName 外设名称
 * @param registerName 寄存器名称
 * @return 寄存器描述，未找到时 name 为空
 */
SvdRegister SvdParser::register_(const QString& peripheralName,
                                  const QString& registerName) const
{
    const SvdPeripheral periph = peripheral(peripheralName);
    if (periph.name.isEmpty()) {
        return SvdRegister{};
    }

    for (const SvdRegister& reg : periph.registers) {
        if (reg.name == registerName) {
            return reg;
        }
    }
    return SvdRegister{};
}

/** @brief 获取最后一次错误信息 */
QString SvdParser::lastError() const
{
    return m_lastError;
}

/** @brief 清除已解析的数据 */
void SvdParser::clear()
{
    m_device = SvdDevice{};
    m_peripheralIndex.clear();
    m_lastError.clear();
}
