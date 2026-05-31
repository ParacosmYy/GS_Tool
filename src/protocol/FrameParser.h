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
 * 状态流转: Idle → HeaderMatching → LengthReceiving → PayloadReceiving → ChecksumVerifying → Done
 *
 * 安全机制:
 *   1. 帧长度上限检查 (maxFrameLength) - 防止超长帧导致内存爆炸
 *   2. 状态机超时机制 - 帧头匹配后超过一定时间未收到完整帧，自动重置状态
 *   3. 硬性上限 kMaxFrameSize - 无论用户如何配置，单帧缓冲区不超过此值
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
     *
     * 默认帧长度上限为 4096 字节，超时阈值为 500ms
     */
    explicit FrameParser(QObject* parent = nullptr);

    /**
     * @brief 设置帧格式定义
     * @param def 帧结构定义（帧头/帧尾/长度字段/校验/字段列表）
     *
     * 设置后会自动 reset() 解析器状态。如果 def.maxFrameLength() 返回有效值，
     * 会更新内部帧长度上限；否则使用硬性上限 kMaxFrameSize
     */
    void setDefinition(const FrameDefinition& def);

    /**
     * @brief 获取当前帧格式定义
     * @return 当前帧结构定义的副本
     */
    FrameDefinition definition() const;

    /**
     * @brief 喂入字节流数据（来自串口/TCP等数据源）
     * @param data 原始字节数据
     *
     * 逐字节送入状态机处理。内部会检查:
     *   - 空数据直接忽略
     *   - 每个字节处理前检查帧长度上限
     *   - 每个字节处理前检查状态机超时
     */
    void feed(const QByteArray& data);

    /**
     * @brief 重置解析器状态
     *
     * 清空缓冲区，回到 Idle 状态，重置帧头匹配进度。
     * 注意: 不会清零 frameCount/errorCount 计数器
     */
    void reset();

    /**
     * @brief 获取已解析的帧计数
     * @return 成功解析的帧总数
     */
    quint64 frameCount() const;

    /**
     * @brief 获取校验失败的帧计数
     * @return 错误帧总数（校验失败 + 超时 + 超长等）
     */
    quint64 errorCount() const;

    /**
     * @brief 设置帧长度上限
     * @param maxLen 最大允许帧长度（字节），必须 > 0
     *
     * 当帧缓冲区大小超过此值时，当前帧将被丢弃并重置状态。
     * 此值不会超过硬性上限 kMaxFrameSize（4096 字节）
     */
    void setMaxFrameLength(int maxLen);

    /**
     * @brief 获取当前帧长度上限
     * @return 最大允许帧长度（字节）
     */
    int maxFrameLength() const;

    /**
     * @brief 设置状态机超时阈值
     * @param timeoutMs 超时时间（毫秒），必须 > 0
     *
     * 当帧头匹配成功后开始计时，如果超过此时间仍未完成整帧接收，
     * 状态机将自动重置并报告超时错误。
     * 设为 0 表示禁用超时机制。
     */
    void setFrameTimeout(int timeoutMs);

    /**
     * @brief 获取当前超时阈值
     * @return 超时时间（毫秒），0 表示禁用
     */
    int frameTimeout() const;

signals:
    /**
     * @brief 帧解析成功
     * @param fields 各字段名→值的映射（如 "温度" → 25.3, "电压" → 3300）
     * @param rawFrame 本帧的原始字节数据
     */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /**
     * @brief 帧解析错误
     * @param reason 错误原因描述（如 "Frame exceeds max size"、"Checksum mismatch"、"Frame timeout"）
     * @param rawFrame 出错时已接收的原始数据（可能不完整）
     */
    void frameError(const QString& reason, const QByteArray& rawFrame);

private:
    /**
     * @brief 解析状态枚举
     *
     * 状态流转:
     *   Idle → HeaderMatching → LengthReceiving → PayloadReceiving → ChecksumVerifying → FooterMatching
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

    /**
     * @brief 处理单个字节
     * @param byte 待处理的字节值
     *
     * 根据当前状态将字节送入对应的处理逻辑。
     * 处理前会检查缓冲区长度上限和超时。
     */
    void processByte(unsigned char byte);

    /**
     * @brief 提取帧内各字段值
     * @param frameData 完整帧数据（包含帧头/帧尾）
     * @return 字段名→值的映射，包含 _rawPayload、_rawFrame、_frameTime 等元数据
     */
    QVariantMap extractFields(const QByteArray& frameData) const;

    /**
     * @brief 计算并验证校验值
     * @param frameData 完整帧数据
     * @return true=校验通过，false=校验失败
     */
    bool verifyChecksum(const QByteArray& frameData) const;

    /**
     * @brief 解析长度字段值
     * @param frameData 包含长度字段的帧数据
     * @return 解析出的长度值，-1 表示解析失败
     */
    int parseLengthField(const QByteArray& frameData) const;

    /**
     * @brief 计算校验值（返回字节数组）
     * @param data 需要计算校验的数据区域
     * @return 校验结果字节数组
     */
    QByteArray computeChecksum(const QByteArray& data) const;

    /**
     * @brief 检查状态机是否超时
     * @return true=已超时需要重置，false=未超时或计时未启动
     *
     * 仅在非 Idle 状态下检查。超时后会发射 frameError 信号。
     */
    bool checkTimeout();

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

    static constexpr int kMaxFrameSize = 4096; ///< 硬性上限：单帧最大长度保护（不可超越）
};

#endif // FRAMEPARSER_H
