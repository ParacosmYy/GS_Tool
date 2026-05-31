#ifndef JUSTFLOATBRIDGE_H
#define JUSTFLOATBRIDGE_H

#include "protocol/IProtocolBridge.h"

#include <QByteArray>
#include <QVariantMap>
#include <QString>
#include <QVector>

// JustFloat协议桥 -- 解析VOFA+兼容的JustFloat字节流
//
// 协议格式:
//   [float1_le][float2_le]...[floatN_le][tail_marker]
//   其中 tail_marker = 0x00 0x00 0x80 0x7f (小端IEEE 754正NaN)
//
// 帧结构:
//   - 每帧由N个4字节小端float组成，紧跟4字节尾部标记
//   - 尾部标记是IEEE 754正NaN的小端表示: 00 00 80 7F
//   - 第一帧自动检测通道数量 = (帧总长度 - 4) / 4
//   - 通道自动命名: "CH1", "CH2", ..., "CHn"
//
// 数据层: 不依赖任何表现层类
class JustFloatBridge : public IProtocolBridge {
    Q_OBJECT

public:
    explicit JustFloatBridge(QObject* parent = nullptr);

    // IProtocolBridge接口实现
    void feed(const QByteArray& data) override;
    void reset() override;
    QString name() const override;

    // 设置固定的通道数量（跳过自动检测阶段）
    // 如果设为0（默认），则通过第一帧自动检测通道数
    void setFixedChannelCount(int count);

    // 获取当前通道数量（自动检测后生效）
    int channelCount() const;

private:
    // 尝试从缓冲区中解析完整的帧
    // 返回: 解析消耗的字节数，0表示没有完整帧
    int tryParseFrame();

    // 解析一帧数据并发射frameParsed信号
    // frameSize: 帧总字节数（含尾部标记）
    void parseAndEmit(int frameSize);

    // 自动检测通道数量（根据第一帧的大小推算）
    // floatPayloadSize: 去掉尾部标记后的float数据字节数
    void autoDetectChannels(int floatPayloadSize);

    QByteArray m_buffer;            // 累积的原始字节缓冲区

    int m_channelCount;             // 通道数量（0=尚未检测）
    bool m_channelsDetected;        // 是否已完成通道检测

    // JustFloat尾部标记: 00 00 80 7F（小端IEEE 754正NaN）
    static constexpr unsigned char kTailMarker[4] = {0x00, 0x00, 0x80, 0x7F};
    static constexpr int kTailSize = 4;              // 尾部标记长度
    static constexpr int kFloatSize = 4;             // 单个float长度
    static constexpr int kMaxBufferSize = 4096;      // 最大缓冲区保护
    static constexpr int kMaxChannels = 32;          // 最大通道数保护
};

#endif // JUSTFLOATBRIDGE_H
