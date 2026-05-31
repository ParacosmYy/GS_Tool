#include "protocol/FireWaterBridge.h"

#include <QStringList>

// ============================================================
// 构造 / 析构
// ============================================================

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

void FireWaterBridge::feed(const QByteArray& data)
{
    if (data.isEmpty()) return;

    // 追加到缓冲区
    m_buffer.append(data);

    // 缓冲区溢出保护: 超过最大限制时丢弃最旧的数据
    if (m_buffer.size() > kMaxBufferSize) {
        int excess = m_buffer.size() - kMaxBufferSize;
        m_buffer.remove(0, excess);
    }

    // 尝试解析所有完整行
    parseLines();
}

void FireWaterBridge::reset()
{
    m_buffer.clear();
    m_channelNames.clear();
    m_headerReceived = false;
    m_firstLineIsData = false;
}

QString FireWaterBridge::name() const
{
    return QStringLiteral("FireWater");
}

// ============================================================
// 配置接口
// ============================================================

void FireWaterBridge::setDelimiter(const QString& delimiter)
{
    m_delimiter = delimiter.isEmpty() ? QStringLiteral(",") : delimiter;
}

QStringList FireWaterBridge::channelNames() const
{
    return m_channelNames;
}

// ============================================================
// 行解析核心逻辑
// ============================================================

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
    emit frameParsed(fields, rawFrame);
}

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
