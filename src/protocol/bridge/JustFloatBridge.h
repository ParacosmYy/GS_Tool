/**
 * @file JustFloatBridge.h
 * @brief JustFloat协议桥 — VOFA+小端浮点字节流解析器接口
 *
 * 解析VOFA+兼容的JustFloat字节流。
 *
 * 协议格式:
 *   [float1_le][float2_le]...[floatN_le][tail_marker]
 *   其中 tail_marker = 0x00 0x00 0x80 0x7f (小端IEEE 754正NaN)
 *
 * 帧结构:
 *   - 每帧由N个4字节小端float组成，紧跟4字节尾部标记
 *   - 尾部标记是IEEE 754正NaN的小端表示: 00 00 80 7F
 *   - 第一帧自动检测通道数量 = (帧总长度 - 4) / 4
 *   - 通道自动命名: "CH1", "CH2", ..., "CHn"
 *
 * 数据层: 不依赖任何表现层类。
 */
#ifndef JUSTFLOATBRIDGE_H
#define JUSTFLOATBRIDGE_H

#include "protocol/bridge/IProtocolBridge.h"

#include <QByteArray>
#include <QVariantMap>
#include <QString>
#include <QVector>

/**
 * @brief JustFloat协议桥 — 解析VOFA+小端浮点字节流
 *
 * 将4字节小端浮点数组+尾部标记的字节流解析为通道数据。
 * 支持自动通道检测和固定通道数配置。
 *
 * 协作关系:
 *   - ProtocolBridgeManager: 创建和管理此桥
 *   - ChartModel: 接收frameParsed信号
 */
class JustFloatBridge : public IProtocolBridge {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit JustFloatBridge(QObject* parent = nullptr);

    /** @brief 喂入原始字节流 @param data 原始数据 */
    void feed(const QByteArray& data) override;
    /** @brief 重置内部状态(清空缓冲区，保留通道配置) */
    void reset() override;
    /** @brief 返回协议名称 @return "JustFloat" */
    QString name() const override;

    /**
     * @brief 设置固定的通道数量(跳过自动检测阶段)
     * @param count 通道数量，设为0(默认)则通过第一帧自动检测
     */
    void setFixedChannelCount(int count);

    /** @brief 获取当前通道数量(自动检测后生效) @return 通道数 */
    int channelCount() const;

    /** @brief 获取已解析的帧计数 */
    quint64 frameCount() const;

    /** @brief 获取解析错误计数 */
    quint64 errorCount() const;

    /** @brief 获取已处理的字节总数 */
    qint64 totalBytesProcessed() const;

    /** @brief 获取已解码通道总数(跨所有帧累加) */
    quint64 totalChannelsDecoded() const;

    /** @brief 获取单帧最大通道数峰值 */
    quint64 peakChannelsPerFrame() const;

    /** @brief 获取累计尾部标记搜索次数(tryParseFrame调用次数) */
    quint64 totalTailSearches() const;
    /** @brief 获取累计对齐错误次数(float数据非4字节对齐) */
    quint64 totalAlignmentErrors() const;
    /** @brief 获取累计通道数不匹配次数(后续帧通道数与首帧不一致) */
    quint64 totalChannelMismatches() const;
    /** @brief 获取累计缓冲区裁剪次数(溢出保护触发) */
    quint64 totalBufferTrims() const;

    /** @brief 重置统计数据（帧计数/错误/字节，不影响通道配置） */
    void resetStatistics();

private:
    /**
     * @brief 尝试从缓冲区中解析完整的帧
     * @return 解析消耗的字节数，0表示没有完整帧
     */
    int tryParseFrame();

    /**
     * @brief 解析一帧数据并发射frameParsed信号
     * @param frameSize 帧总字节数(含尾部标记)
     */
    void parseAndEmit(int frameSize);

    /**
     * @brief 自动检测通道数量(根据第一帧的大小推算)
     * @param floatPayloadSize 去掉尾部标记后的float数据字节数
     */
    void autoDetectChannels(int floatPayloadSize);

    QByteArray m_buffer;            ///< 累积的原始字节缓冲区
    int m_channelCount;             ///< 通道数量(0=尚未检测)
    bool m_channelsDetected;        ///< 是否已完成通道检测

    /** @brief 成功解析帧计数 */
    quint64 m_frameCount = 0;
    /** @brief 错误帧计数 */
    quint64 m_errorCount = 0;
    /** @brief 已处理字节总数 */
    qint64 m_totalBytes = 0;
    /** @brief 已解码通道总数(跨所有帧累加) */
    quint64 m_totalChannelsDecoded = 0;
    /** @brief 单帧最大通道数峰值 */
    quint64 m_peakChannelsPerFrame = 0;
    /** @brief 累计尾部标记搜索次数(tryParseFrame调用次数) */
    quint64 m_totalTailSearches = 0;
    /** @brief 累计对齐错误次数(float数据非4字节对齐) */
    quint64 m_totalAlignmentErrors = 0;
    /** @brief 累计通道数不匹配次数(后续帧通道数与首帧不一致) */
    quint64 m_totalChannelMismatches = 0;
    /** @brief 累计缓冲区裁剪次数(溢出保护触发) */
    quint64 m_totalBufferTrims = 0;

    static constexpr unsigned char kTailMarker[4] = {0x00, 0x00, 0x80, 0x7F}; ///< 尾部标记
    static constexpr int kTailSize = 4;             ///< 尾部标记长度
    static constexpr int kFloatSize = 4;            ///< 单个float长度
    static constexpr int kMaxBufferSize = 4096;     ///< 最大缓冲区保护
    static constexpr int kMaxChannels = 32;         ///< 最大通道数保护
};

#endif // JUSTFLOATBRIDGE_H
