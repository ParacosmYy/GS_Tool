/**
 * @file FireWaterBridge.cpp
 * @brief FireWater协议桥实现 — VOFA+ CSV尾标记协议解析器
 *
 * 将CSV格式数据流（以换行符结尾的浮点数据）解析为通道数据，
 * 兼容VOFA+的FireWater协议规范。
 */
#include "protocol/bridge/FireWaterBridge.h"

#include <QStringList>

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造FireWater协议桥，默认逗号分隔符 @param parent 父对象 */
FireWaterBridge::FireWaterBridge(QObject* parent)
    : IProtocolBridge(parent)
    , m_headerReceived(false)
    , m_firstLineIsData(false)
    , m_delimiter(QStringLiteral(","))
{
}

// ============================================================
// IProtocolBridge接口实现
// ============================================================

/** @brief 喂入原始数据(追加缓冲区→溢出保护→解析完整行) @param data 原始字节流 */
void FireWaterBridge::feed(const QByteArray& data)
{
    if (data.isEmpty()) return;

    // 追加到缓冲区
    m_buffer.append(data);
    m_totalBytes += data.size();

    // 缓冲区溢出保护: 超过最大限制时丢弃最旧的数据
    if (m_buffer.size() > kMaxBufferSize) {
        int excess = m_buffer.size() - kMaxBufferSize;
        m_buffer.remove(0, excess);
    }

    // 尝试解析所有完整行
    parseLines();
}

/** @brief 重置解析器状态(清空缓冲区+通道名+头部标志) */
void FireWaterBridge::reset()
{
    m_buffer.clear();
    m_channelNames.clear();
    m_headerReceived = false;
    m_firstLineIsData = false;
}

/** @brief 获取协议名称 @return "FireWater" */
QString FireWaterBridge::name() const
{
    return QStringLiteral("FireWater");
}

// ============================================================
// 配置接口
// ============================================================

/** @brief 设置CSV分隔符(空字符串默认逗号) @param delimiter 分隔符 */
void FireWaterBridge::setDelimiter(const QString& delimiter)
{
    m_delimiter = delimiter.isEmpty() ? QStringLiteral(",") : delimiter;
}

/** @brief 获取已识别的通道名称列表 @return 通道名QStringList */
QStringList FireWaterBridge::channelNames() const
{
    return m_channelNames;
}

// 行解析核心逻辑(parseLines/processLine/parseValueLine)见 FireWaterBridgeParsing.cpp

// frameCount/errorCount/totalBytesProcessed/totalChannelsDecoded/fireWaterMatches/resetStatistics
// 已移至 FireWaterBridgeStats.cpp
