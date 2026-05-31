/**
 * @file FrameParser.h
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 状态流转: Idle -> HeaderMatching -> LengthReceiving -> PayloadReceiving
 *           -> ChecksumVerifying -> FooterMatching -> Done
 *
 * 安全机制:
 *   1. 帧长度上限检查 (maxFrameLength) - 默认1024字节，超长帧自动丢弃
 *   2. 状态机超时机制 - 帧头匹配后超时自动重置
 *   3. 硬性上限 kMaxFrameSize (4096) - 无论配置如何，单帧不超过此值
 *   4. 帧计数器 - 累计解析帧数/错误帧数，用于诊断和统计
 *   5. 超时恢复 - 超时后清除所有中间缓冲区和匹配进度
 *
 * 设计模式: 状态模式(State) - 解析行为由当前状态驱动，状态转换由接收到的字节决定
 */

#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QTimer>
#include "protocol/FrameDefinition.h"
#include "utils/CRC.h"

/**
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 协作关系:
 *   - ProtocolBridgeManager: 持有 FrameParser，将串口数据路由到 feed()
 *   - FrameDefinition: 定义帧结构（帧头/帧尾/校验/字段布局）
 *   - ChartModel / ProtocolView: 下游消费者，连接 frameParsed/frameError 信号
 */
class FrameParser : public QObject {
    Q_OBJECT

public:
    /** @brief 构造帧解析器，默认帧上限1024字节 */
    explicit FrameParser(QObject* parent = nullptr);
    ~FrameParser() override;

    /** @brief 设置帧格式定义，设置后自动 reset() */
    void setDefinition(const FrameDefinition& def);

    /** @brief 获取当前帧格式定义 */
    FrameDefinition definition() const;

    /** @brief 喂入字节流数据，逐字节送入状态机处理。空数据直接忽略 */
    void feed(const QByteArray& data);

    /** @brief 重置解析器状态（清空缓冲区，回到Idle），不清零计数器 */
    void reset();

    /** @brief 获取已解析的帧计数 */
    quint64 frameCount() const;

    /** @brief 获取错误帧计数 */
    quint64 errorCount() const;

    /**
     * @brief 设置帧长度上限，传入值 <= 0 或 > kMaxFrameSize 时使用 kMaxFrameSize
     * 默认值为 kDefaultMaxFrameLength (1024字节)
     */
    void setMaxFrameLength(int maxLen);

    /** @brief 获取当前帧长度上限 */
    int maxFrameLength() const;

    /** @brief 设置状态机超时阈值（毫秒），0 禁用 */
    void setFrameTimeout(int timeoutMs);

    /** @brief 获取当前超时阈值（毫秒），0 表示禁用 */
    int frameTimeout() const;

signals:
    /** @brief 帧解析成功，fields 为各字段名->值映射 */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief 帧解析错误 */
    void frameError(const QString& reason, const QByteArray& rawFrame);

private:
    /** @brief 解析状态枚举 */
    enum class State {
        Idle,               ///< 等待帧头
        HeaderMatching,     ///< 正在匹配帧头
        LengthReceiving,    ///< 正在接收长度字段
        PayloadReceiving,   ///< 正在接收有效数据
        ChecksumVerifying,  ///< 正在接收校验字段
        FooterMatching      ///< 正在匹配帧尾
    };

    // ---- 状态处理方法 ----
    void processByte(unsigned char byte);
    void handleHeaderMatching(unsigned char byte);
    void handleLengthReceiving(unsigned char byte);
    void handlePayloadReceiving(unsigned char byte);
    void processCompletePayload();   ///< 长度字段模式下帧完成处理(校验→帧尾→完成)
    bool handleCrcValidation();      ///< CRC校验验证，通过返回true
    void handleChecksumVerifying(unsigned char byte);
    void handleFooterMatching(unsigned char byte);

    // ---- 辅助方法（实现在 FrameParserHelpers.cpp） ----
    QVariantMap extractFields(const QByteArray& frameData) const;
    bool verifyChecksum(const QByteArray& frameData) const;
    int parseLengthField(const QByteArray& frameData) const;
    QByteArray computeChecksum(const QByteArray& data) const;

    // ---- 状态机基础设施 ----
    bool checkTimeout();
    void completeFrame();
    void stopTimeoutTimer();
    void startTimeoutTimer();
    void resetIntermediateState();

    // ---- 成员变量 ----
    State m_state = State::Idle;            ///< 当前解析状态
    FrameDefinition m_def;                  ///< 当前帧格式定义
    QByteArray m_buffer;                    ///< 当前帧缓冲区
    int m_headerMatchPos = 0;               ///< 帧头匹配进度
    int m_footerMatchPos = 0;               ///< 帧尾匹配进度
    int m_expectedPayload = 0;              ///< 期望的有效数据长度
    quint64 m_frameCount = 0;               ///< 成功解析帧计数
    quint64 m_errorCount = 0;               ///< 错误帧计数

    int m_maxFrameLength = kDefaultMaxFrameLength; ///< 帧长度上限（默认1024字节）
    int m_frameTimeoutMs = 500;             ///< 帧超时阈值（毫秒），0=禁用
    QElapsedTimer m_frameTimer;             ///< 帧接收计时器
    QTimer* m_timeoutCheckTimer = nullptr;  ///< 独立的超时检查定时器

    static constexpr int kMaxFrameSize = 4096;          ///< 硬性上限
    static constexpr int kDefaultMaxFrameLength = 1024;  ///< 默认帧长度上限
};

#endif // FRAMEPARSER_H
