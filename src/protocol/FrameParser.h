#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include "protocol/FrameDefinition.h"
#include "utils/CRC.h"

/**
 * @file FrameParser.h
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 状态流转: Idle -> HeaderMatching -> LengthReceiving -> PayloadReceiving
 *           -> ChecksumVerifying -> FooterMatching -> Done
 *
 * 安全机制:
 *   1. 帧长度上限检查 (maxFrameLength) - 防止超长帧导致内存爆炸
 *   2. 状态机超时机制 - 帧头匹配后超时自动重置
 *   3. 硬性上限 kMaxFrameSize - 无论配置如何，单帧不超过此值
 */

/**
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 职责:
 *   1. 逐字节接收串口数据，通过状态机匹配帧头/长度/数据/校验/帧尾
 *   2. 解析完成后提取各字段值，通过 frameParsed 信号通知下游
 *   3. 校验失败或格式错误时通过 frameError 信号报告原因
 *
 * 协作关系:
 *   - ProtocolBridgeManager: 持有 FrameParser，将串口数据路由到 feed()
 *   - FrameDefinition: 定义帧结构（帧头/帧尾/校验/字段布局）
 *   - ChartModel / ProtocolView: 下游消费者，连接 frameParsed/frameError 信号
 *
 * 设计模式: 状态模式(State) - 解析行为由当前状态驱动，状态转换由接收到的字节决定
 */
class FrameParser : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造帧解析器
     * @param parent 父对象（通常为 ProtocolBridgeManager）
     */
    explicit FrameParser(QObject* parent = nullptr);

    /** @brief 设置帧格式定义，设置后自动 reset() */
    void setDefinition(const FrameDefinition& def);

    /** @brief 获取当前帧格式定义 */
    FrameDefinition definition() const;

    /**
     * @brief 喂入字节流数据（来自串口/TCP等数据源）
     * @param data 原始字节数据
     *
     * 逐字节送入状态机处理。空数据直接忽略。
     */
    void feed(const QByteArray& data);

    /**
     * @brief 重置解析器状态
     *
     * 清空缓冲区，回到 Idle 状态，重置帧头匹配进度。
     * 不会清零 frameCount/errorCount 计数器。
     */
    void reset();

    /** @brief 获取已解析的帧计数 */
    quint64 frameCount() const;

    /** @brief 获取校验失败的帧计数 */
    quint64 errorCount() const;

    /**
     * @brief 设置帧长度上限
     * @param maxLen 最大允许帧长度（字节），必须 > 0
     *
     * 不会超过硬性上限 kMaxFrameSize（4096 字节）
     */
    void setMaxFrameLength(int maxLen);

    /** @brief 获取当前帧长度上限 */
    int maxFrameLength() const;

    /**
     * @brief 设置状态机超时阈值
     * @param timeoutMs 超时时间（毫秒），设为 0 禁用
     */
    void setFrameTimeout(int timeoutMs);

    /** @brief 获取当前超时阈值（毫秒），0 表示禁用 */
    int frameTimeout() const;

signals:
    /**
     * @brief 帧解析成功
     * @param fields 各字段名->值的映射
     * @param rawFrame 本帧的原始字节数据
     */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /**
     * @brief 帧解析错误
     * @param reason 错误原因描述
     * @param rawFrame 出错时已接收的原始数据
     */
    void frameError(const QString& reason, const QByteArray& rawFrame);

private:
    /**
     * @brief 解析状态枚举
     *
     * 状态流转:
     *   Idle -> HeaderMatching -> LengthReceiving -> PayloadReceiving
     *   -> ChecksumVerifying -> FooterMatching
     *   任何状态下检测到错误都会回到 Idle
     */
    enum class State {
        Idle,               ///< 等待帧头
        HeaderMatching,     ///< 正在匹配帧头
        LengthReceiving,    ///< 正在接收长度字段
        PayloadReceiving,   ///< 正在接收有效数据
        ChecksumVerifying,  ///< 正在接收校验字段
        FooterMatching      ///< 正在匹配帧尾
    };

    // ---- 状态处理方法（从 processByte 拆分） ----

    /** @brief 处理单个字节 - 状态分发器 */
    void processByte(unsigned char byte);

    /** @brief Idle/HeaderMatching 状态: 逐字节匹配帧头序列 */
    void handleHeaderMatching(unsigned char byte);

    /** @brief LengthReceiving 状态: 接收并解析长度字段 */
    void handleLengthReceiving(unsigned char byte);

    /** @brief PayloadReceiving 状态: 接收有效数据载荷 */
    void handlePayloadReceiving(unsigned char byte);

    /** @brief ChecksumVerifying 状态: 接收并验证校验字段 */
    void handleChecksumVerifying(unsigned char byte);

    /** @brief FooterMatching 状态: 匹配帧尾序列 */
    void handleFooterMatching(unsigned char byte);

    // ---- 辅助方法（实现在 FrameParserHelpers.cpp） ----

    /** @brief 提取帧内各字段值 */
    QVariantMap extractFields(const QByteArray& frameData) const;

    /** @brief 计算并验证校验值 */
    bool verifyChecksum(const QByteArray& frameData) const;

    /** @brief 解析长度字段值 */
    int parseLengthField(const QByteArray& frameData) const;

    /** @brief 计算校验值（返回字节数组） */
    QByteArray computeChecksum(const QByteArray& data) const;

    // ---- 状态机基础设施 ----

    /** @brief 检查状态机是否超时，超时后发射 frameError 并 reset() */
    bool checkTimeout();

    /** @brief 帧完成后的通用处理: 提取字段、发射信号、重置状态 */
    void completeFrame();

    // ---- 成员变量 ----

    State m_state = State::Idle;            ///< 当前解析状态
    FrameDefinition m_def;                  ///< 当前帧格式定义
    QByteArray m_buffer;                    ///< 当前帧缓冲区
    int m_headerMatchPos = 0;               ///< 帧头匹配进度（已匹配的字节数）
    int m_expectedPayload = 0;              ///< 期望的有效数据长度（由长度字段解析得出）
    quint64 m_frameCount = 0;               ///< 成功解析帧计数
    quint64 m_errorCount = 0;               ///< 错误帧计数

    int m_maxFrameLength = kMaxFrameSize;   ///< 用户可配置的帧长度上限
    int m_frameTimeoutMs = 500;             ///< 帧超时阈值（毫秒），0=禁用
    QElapsedTimer m_frameTimer;             ///< 帧接收计时器（帧头匹配成功后启动）

    static constexpr int kMaxFrameSize = 4096; ///< 硬性上限：单帧最大长度保护
};

#endif // FRAMEPARSER_H
