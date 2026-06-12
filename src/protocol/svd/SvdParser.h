/**
 * @file SvdParser.h
 * @brief CMSIS SVD(System View Description)文件解析器
 *
 * 解析 CMSIS SVD XML 文件，提取设备→外设→寄存器→字段的层级结构。
 * 使用 QXmlStreamReader 流式解析，支持大型 SVD 文件。
 * 供 SvdRegisterModel 和 SvdRegisterView 显示 MCU 寄存器描述。
 */
#ifndef SVDPARSER_H
#define SVDPARSER_H

#include <QObject>
#include <QString>
#include <QVector>

#include "protocol/svd/SvdTypes.h"

class QXmlStreamReader;

/**
 * @brief CMSIS SVD 文件解析器
 *
 * 将 SVD XML 文件解析为 SvdDevice 数据结构树，支持:
 * - 设备级别属性 (vendor, name, addressUnitBits, width)
 * - 外设列表 (name, baseAddress, registers, clusters)
 * - 寄存器和簇 (name, addressOffset, size, access, resetValue)
 * - 位域字段 (name, bitOffset, bitWidth, enumeratedValues)
 *
 * 使用示例:
 * @code
 * SvdParser parser(this);
 * if (parser.loadSvdFile("STM32F407.svd")) {
 *     auto periph = parser.peripheral("GPIOA");
 *     for (const auto& reg : periph.registers) {
 *         qDebug() << reg.name << reg.addressOffset;
 *     }
 * }
 * @endcode
 */
class SvdParser : public QObject {
    Q_OBJECT

public:
    /** @brief 构造 SVD 解析器 @param parent 父对象 */
    explicit SvdParser(QObject* parent = nullptr);

    // ---- 文件加载 ----

    /**
     * @brief 从文件加载 SVD XML
     * @param filePath SVD 文件路径 (.svd 或 .xml)
     * @return true=解析成功
     */
    bool loadSvdFile(const QString& filePath);

    /**
     * @brief 从 XML 文本内容解析 SVD
     * @param xmlContent SVD XML 字符串
     * @return true=解析成功
     */
    bool loadSvdContent(const QString& xmlContent);

    // ---- 数据查询 ----

    /** @brief 获取已解析的完整设备描述 */
    SvdDevice device() const;

    /** @brief 获取所有外设列表 */
    QVector<SvdPeripheral> peripherals() const;

    /**
     * @brief 按名称查找外设
     * @param name 外设名称 (如 "GPIOA")
     * @return 外设描述，未找到时 name 为空
     */
    SvdPeripheral peripheral(const QString& name) const;

    /**
     * @brief 按外设名+寄存器名查找寄存器
     * @param peripheralName 外设名称
     * @param registerName 寄存器名称
     * @return 寄存器描述，未找到时 name 为空
     */
    SvdRegister register_(const QString& peripheralName,
                          const QString& registerName) const;

    // ---- 统计接口 (实现在 SvdParserStats.cpp) ----

    quint64 totalPeripherals() const;  ///< 累计解析外设总数
    quint64 totalRegisters() const;    ///< 累计解析寄存器总数
    quint64 totalFields() const;       ///< 累计解析字段总数
    quint64 totalParseErrors() const;  ///< 累计解析错误总数
    void resetStatistics();            ///< 重置所有统计计数器

    /** @brief 获取最后一次错误信息 */
    QString lastError() const;

    /** @brief 清除已解析的数据 */
    void clear();

signals:
    /** @brief SVD 文件解析完成 @param peripheralCount 外设数量 */
    void parseCompleted(int peripheralCount);

    /** @brief 解析错误 @param errorMessage 错误描述 */
    void parseError(const QString& errorMessage);

private:
    // ---- XML 解析方法 ----
    bool parseDevice(QXmlStreamReader& xml);
    void parsePeripheral(QXmlStreamReader& xml);
    void parseRegister(QXmlStreamReader& xml, QVector<SvdRegister>& out);
    void parseCluster(QXmlStreamReader& xml, QVector<SvdCluster>& out);
    void parseField(QXmlStreamReader& xml, QVector<SvdField>& out);
    void parseEnumeratedValues(QXmlStreamReader& xml, QVector<SvdEnumeratedValue>& out);

    /** @brief 从十六进制字符串解析地址值 */
    quint64 parseHexValue(const QString& value) const;

    // ---- 成员 ----
    SvdDevice           m_device;           ///< 已解析的设备描述
    SvdParseStatistics  m_stats;            ///< 解析统计
    QString             m_lastError;        ///< 最后错误信息
    QMap<QString, int>  m_peripheralIndex;  ///< 外设名→索引查找表
};

#endif // SVDPARSER_H
