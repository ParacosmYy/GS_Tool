/**
 * @file DataStreamSplitter.cpp
 * @brief 数据流分割器核心实现
 *
 * 实现4种分割模式的缓冲区管理和帧提取逻辑。
 * 内部缓冲区累积原始字节流，根据当前模式切割完整帧。
 */

#include "utils/splitter/DataStreamSplitter.h"

#include <QFile>
#include <QSaveFile>
#include <QDateTime>
#include <algorithm>
#include <cstring>

// ===========================================================================
// 构造 / 析构
// ===========================================================================

/** @brief 构造数据流分割器，初始化超时定时器 */
DataStreamSplitter::DataStreamSplitter(QObject* parent)
    : QObject(parent)
    , m_frameIndex(0)
    , m_timeoutTimer(new QTimer(this))
{
    memset(&m_stats, 0, sizeof(m_stats));
    setObjectName(QStringLiteral("DataStreamSplitter"));

    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &DataStreamSplitter::onTimeout);
}

/** @brief 析构时确保超时定时器停止 */
DataStreamSplitter::~DataStreamSplitter()
{
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}

// ===========================================================================
// 规则配置
// ===========================================================================

/**
 * @brief 设置分割规则
 *
 * 切换规则时清空缓冲区、重置帧计数、停止超时定时器。
 * HeaderLength模式下会校验headerLenSize合法性(1/2/4)。
 */
void DataStreamSplitter::setRule(const SplitRule& rule)
{
    // HeaderLength模式参数校验
    if (rule.mode == SplitMode::HeaderLength) {
        if (rule.headerLenSize != 1 && rule.headerLenSize != 2
            && rule.headerLenSize != 4) {
            emit splitterError(tr("HeaderLength模式: headerLenSize必须为1/2/4"));
            return;
        }
        if (rule.headerLenPos < 0) {
            emit splitterError(tr("HeaderLength模式: headerLenPos不能为负数"));
            return;
        }
    }

    // FixedLength模式参数校验
    if (rule.mode == SplitMode::FixedLength && rule.fixedLength <= 0) {
        emit splitterError(tr("FixedLength模式: fixedLength必须大于0"));
        return;
    }

    // Delimiter模式参数校验
    if (rule.mode == SplitMode::Delimiter && rule.delimiter.isEmpty()) {
        emit splitterError(tr("Delimiter模式: 分隔符不能为空"));
        return;
    }

    // Timeout模式参数校验
    if (rule.mode == SplitMode::Timeout && rule.timeoutMs <= 0) {
        emit splitterError(tr("Timeout模式: 超时时间必须大于0"));
        return;
    }

    // 停止旧定时器并清空状态
    m_timeoutTimer->stop();
    m_buffer.clear();
    m_frameIndex = 0;

    m_rule = rule;
}

/** @brief 获取当前分割规则 */
const DataStreamSplitter::SplitRule& DataStreamSplitter::rule() const
{
    return m_rule;
}

// ===========================================================================
// 数据处理
// ===========================================================================

/**
 * @brief 输入原始字节流，提取完整帧
 *
 * 数据追加到内部缓冲区后，根据当前模式分发到对应分割函数。
 * 每提取一帧都通过 frameAvailable 信号通知外部。
 */
QList<DataStreamSplitter::SplitResult> DataStreamSplitter::process(
    const QByteArray& data)
{
    QList<SplitResult> results;

    if (data.isEmpty()) {
        return results;
    }

    // 追加到缓冲区
    m_buffer.append(data);
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());

    // 防止缓冲区无限增长(单次最大10MB)
    static const qsizetype kMaxBufferSize = 10 * 1024 * 1024;
    if (m_buffer.size() > kMaxBufferSize) {
        quint64 discarded = static_cast<quint64>(m_buffer.size());
        m_stats.totalDiscarded += discarded;
        m_stats.splitErrors++;
        emit splitterError(tr("缓冲区溢出，已丢弃 %1 字节")
                           .arg(discarded));
        m_buffer.clear();
        return results;
    }

    // 按模式分发
    switch (m_rule.mode) {
    case SplitMode::Delimiter:
        splitByDelimiter(results);
        break;
    case SplitMode::FixedLength:
        splitByFixedLength(results);
        break;
    case SplitMode::HeaderLength:
        splitByHeaderLength(results);
        break;
    case SplitMode::Timeout:
        // 超时模式: 启动/重启定时器，不主动分割
        m_timeoutTimer->start(m_rule.timeoutMs);
        m_lastDataTime.start();
        break;
    }

    // 发射每帧信号
    for (const SplitResult& frame : results) {
        emit frameAvailable(frame);
    }

    return results;
}

// ===========================================================================
// 分隔符模式
// ===========================================================================

/**
 * @brief 分隔符模式分割
 *
 * 在缓冲区中反复搜索分隔符，每次找到后提取分隔符前的数据为一帧。
 * 分隔符本身不包含在输出帧中。
 */
void DataStreamSplitter::splitByDelimiter(QList<SplitResult>& results)
{
    const QByteArray& sep = m_rule.delimiter;
    if (sep.isEmpty()) return;

    int sepLen = sep.size();
    int idx = m_buffer.indexOf(sep);

    while (idx >= 0) {
        // 提取分隔符前的数据(可能为空帧)
        QByteArray frameData = m_buffer.left(idx);
        m_buffer.remove(0, idx + sepLen);

        if (!frameData.isEmpty()) {
            results.append(makeResult(frameData));
        }
        // 空帧不计入统计，但继续搜索

        idx = m_buffer.indexOf(sep);
    }
}

// ===========================================================================
// 固定长度模式
// ===========================================================================

/**
 * @brief 固定长度模式分割
 *
 * 缓冲区数据量 >= fixedLength 时，每次切割 fixedLength 字节为一帧。
 * 重复切割直到缓冲区不足一个完整帧。
 */
void DataStreamSplitter::splitByFixedLength(QList<SplitResult>& results)
{
    int len = m_rule.fixedLength;
    if (len <= 0) return;

    while (m_buffer.size() >= len) {
        QByteArray frameData = m_buffer.left(len);
        m_buffer.remove(0, len);
        results.append(makeResult(frameData));
    }
}

// ===========================================================================
// 报头长度模式
// ===========================================================================

/**
 * @brief 报头长度模式分割
 *
 * 从缓冲区 headerLenPos 处读取 headerLenSize 字节作为长度值(大端序)，
 * 实际帧总长 = 长度值 + headerLenOffset。
 * 缓冲区数据量达到总帧长时切割。
 * 如果读取到的长度值异常(<=0 或 > 10MB)，丢弃首字节并报告错误。
 */
void DataStreamSplitter::splitByHeaderLength(QList<SplitResult>& results)
{
    static const int kMaxFrameSize = 10 * 1024 * 1024;

    while (!m_buffer.isEmpty()) {
        int pos = m_rule.headerLenPos;
        int size = m_rule.headerLenSize;

        // 检查缓冲区是否有足够的报头字节可读
        if (m_buffer.size() < pos + size) {
            break;  // 报头不完整，等待更多数据
        }

        int lengthValue = readLengthField(pos, size);
        if (lengthValue < 0) {
            // 读取失败(不应发生)，跳过一字节
            m_stats.totalDiscarded++;
            m_stats.splitErrors++;
            emit splitterError(tr("HeaderLength模式: 长度字段读取失败"));
            m_buffer.remove(0, 1);
            continue;
        }

        int totalFrameLen = lengthValue + m_rule.headerLenOffset;

        // 合法性校验
        if (totalFrameLen <= 0 || totalFrameLen > kMaxFrameSize) {
            m_stats.totalDiscarded++;
            m_stats.splitErrors++;
            emit splitterError(
                tr("HeaderLength模式: 异常帧长度 %1 (原始值=%2, 偏移=%3)")
                .arg(totalFrameLen).arg(lengthValue).arg(m_rule.headerLenOffset));
            m_buffer.remove(0, 1);
            continue;
        }

        // 缓冲区数据不足以组成完整帧
        if (m_buffer.size() < totalFrameLen) {
            break;
        }

        QByteArray frameData = m_buffer.left(totalFrameLen);
        m_buffer.remove(0, totalFrameLen);
        results.append(makeResult(frameData));
    }
}

// ===========================================================================
// 超时模式
// ===========================================================================

/**
 * @brief 超时定时器回调
 *
 * 定时器触发时，如果缓冲区非空则将全部内容作为一帧输出。
 * 适用于不定长协议或字符终端的行缓冲场景。
 */
void DataStreamSplitter::onTimeout()
{
    if (m_buffer.isEmpty()) return;

    QByteArray frameData;
    frameData.swap(m_buffer);  // 零拷贝取走缓冲区内容
    m_buffer.clear();

    SplitResult result = makeResult(frameData);
    emit frameAvailable(result);
}

// ===========================================================================
// 重置 / 查询
// ===========================================================================

/** @brief 清空缓冲区并重置帧计数器，不影响规则和统计 */
void DataStreamSplitter::reset()
{
    m_timeoutTimer->stop();
    m_buffer.clear();
    m_frameIndex = 0;
}

/** @brief 获取当前缓冲区内容的副本 */
QByteArray DataStreamSplitter::currentBuffer() const
{
    return m_buffer;
}

// ===========================================================================
// 导出
// ===========================================================================

/**
 * @brief 将帧列表导出为CSV文件
 *
 * CSV格式: 帧序号, 时间戳(epoch ms), 帧长度, Hex数据
 * 使用QSaveFile保证原子写入。
 */
bool DataStreamSplitter::exportFrames(const QList<SplitResult>& frames,
                                       const QString& filePath) const
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // CSV表头
    file.write("FrameIndex,Timestamp,Length,HexData\n");

    for (const SplitResult& frame : frames) {
        QString line = QStringLiteral("%1,%2,%3,%4\n")
            .arg(frame.frameIndex)
            .arg(frame.timestamp)
            .arg(frame.data.size())
            .arg(QString::fromUtf8(frame.data.toHex(' ')));
        file.write(line.toUtf8());
    }

    if (!file.commit()) {
        return false;
    }
    return true;
}

// ===========================================================================
// 内部辅助
// ===========================================================================

/**
 * @brief 从缓冲区读取长度字段(大端序)
 *
 * 支持1/2/4字节宽度，读取位置为pos开始的size字节。
 * @return 长度值，size非法时返回-1
 */
int DataStreamSplitter::readLengthField(int pos, int size) const
{
    if (pos + size > m_buffer.size()) return -1;

    const char* d = m_buffer.constData() + pos;
    switch (size) {
    case 1:
        return static_cast<unsigned char>(d[0]);
    case 2: {
        quint16 val = 0;
        memcpy(&val, d, 2);
        return static_cast<int>((val >> 8) | (val << 8));  // 大端转小端
    }
    case 4: {
        quint32 val = 0;
        memcpy(&val, d, 4);
        // 大端转小端: 逐字节交换
        val = ((val & 0xFF000000u) >> 24)
            | ((val & 0x00FF0000u) >> 8)
            | ((val & 0x0000FF00u) << 8)
            | ((val & 0x000000FFu) << 24);
        return static_cast<int>(val);
    }
    default:
        return -1;
    }
}

/**
 * @brief 更新平均帧大小(滑动平均)
 *
 * 使用加权公式: avg = avg * (n-1)/n + newSize/n
 */
void DataStreamSplitter::updateAvgFrameSize(quint64 frameSize)
{
    quint64 n = m_stats.totalFramesSplit;
    if (n == 0) {
        m_stats.avgFrameSize = frameSize;
    } else {
        m_stats.avgFrameSize = (m_stats.avgFrameSize * (n - 1) + frameSize) / n;
    }
}

/**
 * @brief 构造分割结果并更新统计
 *
 * @param data 帧数据
 * @return 填充了frameIndex/timestamp的SplitResult
 */
DataStreamSplitter::SplitResult DataStreamSplitter::makeResult(
    const QByteArray& data)
{
    SplitResult result;
    result.frameIndex = m_frameIndex++;
    result.data = data;
    result.timestamp = QDateTime::currentMSecsSinceEpoch();

    m_stats.totalFramesSplit++;
    updateAvgFrameSize(static_cast<quint64>(data.size()));

    return result;
}
