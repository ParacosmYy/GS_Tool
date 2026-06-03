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

// ============================================================
// 行解析核心逻辑
// ============================================================

/** @brief 从缓冲区提取所有完整行(\n结尾)并逐行解析 */
void FireWaterBridge::parseLines()
{
    // 循环提取所有以\n结尾的完整行
    while (true) {
        int newlinePos = m_buffer.indexOf('\n');
        if (newlinePos < 0) {
            break;  // 没有完整行
        }

        // 提取一行数据（去掉\n）
        // 同时检查是否存在\r\n结尾（Windows风格）
        int lineEnd = newlinePos;
        QByteArray lineData = m_buffer.left(lineEnd);

        // 去掉可能的\r
        if (lineData.endsWith('\r')) {
            lineData.chop(1);
        }

        // 从缓冲区移除这一行（含\n）
        m_buffer.remove(0, newlinePos + 1);

        // 行长度保护: 超长行可能是垃圾数据
        if (lineData.size() > kMaxLineSize) {
            ++m_errorCount;
            continue;
        }

        // 转为QString处理
        QString line = QString::fromUtf8(lineData);
        if (line.isEmpty()) {
            continue;
        }

        // 处理这一行
        processLine(line);
    }
}

/** @brief 处理单行数据(头部识别→通道名初始化→数据解析→发射frameParsed) @param line 一行CSV文本 */
void FireWaterBridge::processLine(const QString& line)
{
    // 如果尚未接收到头部行，需要判断这是头部还是数据
    if (!m_headerReceived && !m_firstLineIsData) {
        // 按分隔符分割
        QStringList tokens = line.split(m_delimiter, Qt::SkipEmptyParts);
        if (tokens.isEmpty()) {
            return;
        }

        // 判断第一个token是否为数字
        bool firstIsNumber = false;
        tokens.first().trimmed().toDouble(&firstIsNumber);

        if (firstIsNumber) {
            // 第一行是数据行 -- 自动生成通道名
            m_firstLineIsData = true;
            int count = tokens.size();
            if (count > kMaxChannels) {
                count = kMaxChannels;
            }
            for (int i = 0; i < count; ++i) {
                m_channelNames.append(QStringLiteral("CH%1").arg(i + 1));
            }
        } else {
            // 第一行是头部行 -- 使用token作为通道名称
            m_headerReceived = true;
            int count = tokens.size();
            if (count > kMaxChannels) {
                count = kMaxChannels;
            }
            for (int i = 0; i < count; ++i) {
                m_channelNames.append(tokens[i].trimmed());
            }
            return;  // 头部行不发射数据信号
        }
    }

    // 解析数据行
    QVariantMap fields = parseValueLine(line);
    if (fields.isEmpty()) {
        return;
    }

    // 确保通道名称列表已初始化（如果第一行是数据，上面已初始化）
    if (m_channelNames.isEmpty()) {
        return;
    }

    // 构造原始帧数据（行内容 + \n）
    QByteArray rawFrame = (line + QStringLiteral("\n")).toUtf8();

    // 发射与FrameParser::frameParsed完全兼容的信号
    ++m_frameCount;
    emit frameParsed(fields, rawFrame);
}

/** @brief 将CSV数据行解析为通道名→浮点值映射(非数字用qNaN占位) @param line CSV数据行 @return 通道名→值QVariantMap */
QVariantMap FireWaterBridge::parseValueLine(const QString& line) const
{
    QStringList tokens = line.split(m_delimiter, Qt::SkipEmptyParts);
    if (tokens.isEmpty()) {
        return QVariantMap();
    }

    // 限制token数量不超过通道数
    int count = qMin(tokens.size(), m_channelNames.size());
    if (count == 0) {
        return QVariantMap();
    }

    QVariantMap fields;
    for (int i = 0; i < count; ++i) {
        bool ok = false;
        double value = tokens[i].trimmed().toDouble(&ok);
        if (ok) {
            fields[m_channelNames[i]] = value;
        } else {
            // 非数字值，跳过但保留通道占位
            // 使用NaN表示解析失败的通道
            fields[m_channelNames[i]] = qQNaN();
        }
    }

    return fields;
}

/**
 * @brief 获取已解析的帧计数
 */
quint64 FireWaterBridge::frameCount() const
{
    return m_frameCount;
}

/**
 * @brief 获取解析错误计数
 */
quint64 FireWaterBridge::errorCount() const
{
    return m_errorCount;
}

/**
 * @brief 获取已处理的字节总数
 */
qint64 FireWaterBridge::totalBytesProcessed() const
{
    return m_totalBytes;
}

/**
 * @brief 重置统计数据
 */
void FireWaterBridge::resetStatistics()
{
    m_frameCount = 0;
    m_errorCount = 0;
    m_totalBytes = 0;
}
