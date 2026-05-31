#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include "protocol/FrameDefinition.h"
#include "utils/CRC.h"

// 帧解析状态机 - 从字节流中实时解析协议帧
// 状态流转: Idle → HeaderMatching → LengthReceiving → PayloadReceiving → ChecksumVerifying → Done
class FrameParser : public QObject {
    Q_OBJECT

public:
    explicit FrameParser(QObject* parent = nullptr);

    // 设置帧格式定义
    void setDefinition(const FrameDefinition& def);

    // 获取当前帧格式定义
    FrameDefinition definition() const;

    // 喂入字节流数据（来自串口/TCP等数据源）
    void feed(const QByteArray& data);

    // 重置解析器状态
    void reset();

    // 获取已解析的帧计数
    quint64 frameCount() const;

    // 获取校验失败的帧计数
    quint64 errorCount() const;

signals:
    // 帧解析成功，fields包含各字段名=值
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // 帧解析错误
    void frameError(const QString& reason, const QByteArray& rawFrame);

private:
    // 解析状态枚举
    enum class State {
        Idle,               // 等待帧头
        HeaderMatching,     // 正在匹配帧头
        LengthReceiving,    // 正在接收长度字段
        PayloadReceiving,   // 正在接收有效数据
        ChecksumVerifying,  // 正在接收校验字段
        FooterMatching      // 正在匹配帧尾
    };

    // 处理单个字节
    void processByte(unsigned char byte);

    // 提取帧内各字段值
    QVariantMap extractFields(const QByteArray& frameData) const;

    // 计算并验证校验值
    bool verifyChecksum(const QByteArray& frameData) const;

    // 解析长度字段值
    int parseLengthField(const QByteArray& frameData) const;

    // 计算校验值（返回字节数组）
    QByteArray computeChecksum(const QByteArray& data) const;

    State m_state = State::Idle;
    FrameDefinition m_def;
    QByteArray m_buffer;        // 当前帧缓冲
    int m_headerMatchPos = 0;   // 帧头匹配进度
    int m_expectedPayload = 0;  // 期望的有效数据长度
    quint64 m_frameCount = 0;
    quint64 m_errorCount = 0;

    static constexpr int kMaxFrameSize = 4096; // 单帧最大长度保护
};

#endif // FRAMEPARSER_H
