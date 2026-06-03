/**
 * @file FrameParser.cpp
 * @brief 帧解析状态机实现 - 状态分发与各状态处理方法
 *
 * 容错性增强:
 *   - 默认帧长度上限 1024 字节（kDefaultMaxFrameLength）
 *   - 超时后 resetIntermediateState() 完整清除所有中间状态
 *   - 帧计数器用于诊断和统计
 *
 * 辅助方法实现在 FrameParserHelpers.cpp 中。
 * 状态机处理方法实现在 FrameParserStateHandlers.cpp 中。
 */

#include "protocol/parser/FrameParser.h"
#include <QDebug>

// ============================================================================
// 构造 / 配置 / 状态查询
// ============================================================================

/** @brief 构造帧解析器(创建超时检查定时器) @param parent 父对象 */
FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
    , m_timeoutCheckTimer(new QTimer(this))
{
    m_timeoutCheckTimer->setSingleShot(false);
    connect(m_timeoutCheckTimer, &QTimer::timeout,
            this, [this]() { checkTimeout(); });
}

FrameParser::~FrameParser()
{
    stopTimeoutTimer();
}

/** @brief 设置帧定义(配置帧头/长度/CRC/帧尾等解析规则) @param def 帧定义 */
void FrameParser::setDefinition(const FrameDefinition& def)
{
    m_def = def;
    reset();

    int defMaxLen = m_def.maxFrameLength();
    if (defMaxLen > 0 && defMaxLen <= kMaxFrameSize) {
        m_maxFrameLength = defMaxLen;
    }

    startTimeoutTimer();
}

/** @brief 获取当前帧定义 @return FrameDefinition副本 */
FrameDefinition FrameParser::definition() const
{
    return m_def;
}

/** @brief 输入原始字节流进行帧解析(逐字节状态机) @param data 原始字节 */
void FrameParser::feed(const QByteArray& data)
{
    if (data.isEmpty()) {
        return;
    }

    /* 累计输入字节统计 */
    m_totalBytesInput += static_cast<quint64>(data.size());

    for (int i = 0; i < data.size(); ++i) {
        processByte(static_cast<unsigned char>(data[i]));
    }
}

/** @brief 重置解析器状态（清空缓冲区，回到Idle），不清零计数器 */
void FrameParser::reset()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_headerMatchPos = 0;
    m_expectedPayload = 0;
    m_footerMatchPos = 0;
    m_frameTimer.invalidate();
}

/**
 * @brief 完整重置所有中间缓冲区和匹配进度（超时恢复用）
 *
 * 比 reset() 更彻底，额外调用 squeeze() 释放内存，
 * 确保下一帧从完全干净的状态开始解析。
 */
/** @brief 重置解析器中间状态(不清零统计计数器) */
void FrameParser::resetIntermediateState()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_buffer.squeeze();
    m_headerMatchPos = 0;
    m_footerMatchPos = 0;
    m_expectedPayload = 0;
    m_frameTimer.invalidate();
}

/** @brief 获取已成功解析的帧总数 @return 帧计数 */
quint64 FrameParser::frameCount() const { return m_frameCount; }
/** @brief 获取解析错误计数(CRC错/超时/格式错) @return 错误计数 */
quint64 FrameParser::errorCount() const { return m_errorCount; }

/** @brief 设置最大帧长度(超过则视为错误帧) @param maxLen 最大字节数 */
void FrameParser::setMaxFrameLength(int maxLen)
{
    m_maxFrameLength = (maxLen <= 0 || maxLen > kMaxFrameSize) ? kMaxFrameSize : maxLen;
}

/** @brief 返回最大帧长度 @return 字节数 */
int FrameParser::maxFrameLength() const { return m_maxFrameLength; }

/** @brief 设置帧超时时间(接收中途停止超过此时间视为超时) @param timeoutMs 超时毫秒数 */
void FrameParser::setFrameTimeout(int timeoutMs)
{
    m_frameTimeoutMs = (timeoutMs < 0) ? 0 : timeoutMs;
    if (m_frameTimeoutMs > 0) {
        startTimeoutTimer();
    } else {
        stopTimeoutTimer();
    }
}

/** @brief 返回帧超时时间 @return 毫秒数 */
int FrameParser::frameTimeout() const { return m_frameTimeoutMs; }

// ============================================================================
// 统计计数器接口
// ============================================================================

/** @brief 获取成功解析的帧总数 @return 累计帧数 */
quint64 FrameParser::totalFramesParsed() const { return m_totalFramesParsed; }

/** @brief 获取累计输入的字节总数 @return 字节数 */
quint64 FrameParser::totalBytesInput() const { return m_totalBytesInput; }

/** @brief 获取校验和错误次数(CRC/Sum/异或不匹配) @return 错误次数 */
quint64 FrameParser::totalChecksumErrors() const { return m_totalChecksumErrors; }

/** @brief 获取累计溢出次数（帧超过最大长度被丢弃） @return 溢出次数 */
quint64 FrameParser::totalOverflows() const { return m_totalOverflows; }

/** @brief 重置所有统计计数器(帧数/字节/校验错误/溢出) */
void FrameParser::resetStats()
{
    m_frameCount = 0;
    m_errorCount = 0;
    m_totalFramesParsed = 0;
    m_totalBytesInput = 0;
    m_totalChecksumErrors = 0;
    m_totalOverflows = 0;
}

// ============================================================================
// 状态机基础设施
// ============================================================================

/** @brief 检查是否超时，超时后 emit frameError 并完整重置 */
bool FrameParser::checkTimeout()
{
    if (m_frameTimeoutMs <= 0 || !m_frameTimer.isValid() || m_state == State::Idle) {
        return false;
    }

    if (m_frameTimer.hasExpired(m_frameTimeoutMs)) {
        QByteArray discarded = m_buffer;
        quint64 discardedSize = static_cast<quint64>(m_buffer.size());
        resetIntermediateState();
        m_errorCount++;

        emit frameError(
            tr("帧超时: 已接收 %1 字节 (%2ms)，不完整帧已丢弃")
                .arg(discardedSize).arg(m_frameTimeoutMs),
            discarded);

        stopTimeoutTimer();
        startTimeoutTimer();
        return true;
    }
    return false;
}

/** @brief 停止超时定时器(帧完成或重置时调用) */
void FrameParser::stopTimeoutTimer()
{
    if (m_timeoutCheckTimer) {
        m_timeoutCheckTimer->stop();
    }
}

/** @brief 启动超时定时器(开始接收帧数据时调用) */
void FrameParser::startTimeoutTimer()
{
    if (m_timeoutCheckTimer && m_frameTimeoutMs > 0) {
        int interval = qBound(50, m_frameTimeoutMs / 2, 500);
        m_timeoutCheckTimer->start(interval);
    }
}

/** @brief 帧接收完成：提取字段、发射frameParsed信号、更新统计、重置状态 */
void FrameParser::completeFrame()
{
    QVariantMap fields = extractFields(m_buffer);
    m_frameCount++;
    m_totalFramesParsed++;
    emit frameParsed(fields, m_buffer);
    reset();
}

// 状态机处理方法（processByte / handleXxx）实现在 FrameParserStateHandlers.cpp 中。
