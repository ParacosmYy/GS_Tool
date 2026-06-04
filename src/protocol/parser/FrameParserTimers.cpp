/**
 * @file FrameParserTimers.cpp
 * @brief 帧解析器超时定时器管理与统计计数器查询接口
 *
 * 从 FrameParser.cpp 拆分而来，包含:
 *   - 统计计数器getter(totalFramesParsed/totalBytesInput/totalChecksumErrors/...)
 *   - resetStats()
 *   - 超时检查(checkTimeout)
 *   - 定时器启停(startTimeoutTimer/stopTimeoutTimer)
 *   - 帧完成处理(completeFrame)
 *
 * 构造/析构/配置/feed/reset见 FrameParser.cpp。
 * 状态机处理方法见 FrameParserStateHandlers.cpp。
 * 校验与帧尾处理方法见 FrameParserStats.cpp。
 */

#include "protocol/parser/FrameParser.h"
#include <QDebug>

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

/** @brief 获取累计解析错误次数(含格式错/长度错/帧尾不匹配/超时) @return 错误次数 */
quint64 FrameParser::totalParseErrors() const { return m_totalParseErrors; }

/** @brief 获取成功解析帧中的有效数据字节总数 @return 字节数 */
quint64 FrameParser::totalBytesParsed() const { return m_totalBytesParsed; }

/** @brief 获取累计同步丢失次数(帧头匹配失败导致缓冲区清空) @return 同步丢失次数 */
quint64 FrameParser::totalSyncLost() const { return m_totalSyncLost; }

/** @brief 获取累计帧构建完成次数(completeFrame调用) @return 帧构建次数 */
quint64 FrameParser::totalFramesBuilt() const { return m_totalFramesBuilt; }

/** @brief 获取累计校验验证失败次数(CRC/校验和不匹配) @return 验证失败次数 */
quint64 FrameParser::totalValidationErrors() const { return m_totalValidationErrors; }

/** @brief 获取累计自动检测调用次数(为ProtocolBridgeManager预留) @return 自动检测调用次数 */
quint64 FrameParser::totalAutoDetectCalls() const { return m_totalAutoDetectCalls; }

/** @brief 获取累计畸形帧次数(帧头/帧尾/长度异常) @return 畸形帧次数 */
quint64 FrameParser::totalMalformedFrames() const { return m_totalMalformedFrames; }

/** @brief 获取平均帧大小(字节) @return 平均字节数，无帧时返回0 */
double FrameParser::avgFrameSize() const { return m_avgFrameSize; }

/** @brief 重置所有统计计数器(帧数/字节/校验错误/溢出/解析错误/已解析字节/同步丢失/帧构建/验证错误/自动检测/畸形帧/平均帧大小) */
void FrameParser::resetStats()
{
    m_frameCount = 0;
    m_errorCount = 0;
    m_totalFramesParsed = 0;
    m_totalBytesInput = 0;
    m_totalChecksumErrors = 0;
    m_totalOverflows = 0;
    m_totalParseErrors = 0;
    m_totalBytesParsed = 0;
    m_totalSyncLost = 0;
    m_totalFramesBuilt = 0;
    m_totalValidationErrors = 0;
    m_totalAutoDetectCalls = 0;
    m_totalMalformedFrames = 0;
    m_avgFrameSize = 0.0;
}

// ============================================================================
// 超时检查与定时器管理
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
        ++m_totalParseErrors;  // 帧超时
        ++m_totalMalformedFrames;  ///< 统计: 超时不完整帧视为畸形帧

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
    m_totalFramesBuilt++;
    m_totalBytesParsed += static_cast<quint64>(m_buffer.size());  // 累计已解析字节

    // 统计: 增量更新平均帧大小(指数移动平均，alpha=0.1)
    {
        double currentSize = static_cast<double>(m_buffer.size());
        if (m_totalFramesParsed == 1) {
            m_avgFrameSize = currentSize;
        } else {
            m_avgFrameSize = 0.9 * m_avgFrameSize + 0.1 * currentSize;
        }
    }

    emit frameParsed(fields, m_buffer);
    reset();
}
