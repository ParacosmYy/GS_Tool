#ifndef IPROTOCOLBRIDGE_H
#define IPROTOCOLBRIDGE_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QString>

// 协议桥接口 -- 将原始串口字节流转换为frameParsed兼容的信号
// 实现此接口的桥可以无缝接入现有的ChartModel/ProtocolView管道
// 与FrameParser的frameParsed信号完全兼容，下游组件无需知道数据来自哪个协议
//
// 设计模式: 策略模式(Strategy) -- 不同的协议桥是可互换的策略
// 数据层: 不依赖任何表现层类，纯字节解析 + 信号发射
class IProtocolBridge : public QObject {
    Q_OBJECT

public:
    explicit IProtocolBridge(QObject* parent = nullptr)
        : QObject(parent) {}

    ~IProtocolBridge() override = default;

    // 禁止拷贝和赋值（QObject派生类）
    IProtocolBridge(const IProtocolBridge&) = delete;
    IProtocolBridge& operator=(const IProtocolBridge&) = delete;

    // 喂入原始字节流数据（来自串口/TCP等数据源）
    // 桥在内部累积字节、检测帧边界、解析通道数据
    virtual void feed(const QByteArray& data) = 0;

    // 重置桥的内部状态（清空缓冲区、通道配置等）
    virtual void reset() = 0;

    // 返回协议桥的名称（用于UI显示和日志记录）
    virtual QString name() const = 0;

signals:
    // 与FrameParser::frameParsed完全相同的签名
    // fields: 通道名→值的映射（如 "CH1" → 3.14, "CH2" → 2.72）
    // rawFrame: 本帧的原始字节数据（用于ProtocolView的HEX显示）
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);
};

#endif // IPROTOCOLBRIDGE_H
