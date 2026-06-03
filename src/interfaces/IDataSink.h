/**
 * @file IDataSink.h
 * @brief 数据接收器接口 - 数据流消费者的统一协议
 *
 * 定义了数据流的消费接口，用于终端显示、波形绘制、协议解析等
 * 需要接收数据的模块。实现了观察者模式，数据源通过此接口推送数据。
 *
 * 零出站依赖: 仅依赖 Qt Core 类型，不 include 任何项目头文件
 *
 * 设计模式:
 *   - 观察者模式: 数据源持有 IDataSink 列表，数据到达时通知所有消费者
 *   - 管道模式: 多个 IDataSink 可串联形成处理管道
 *
 * 协作关系:
 *   - IConnection: 数据源，收到数据后分发给所有注册的 IDataSink
 *   - TerminalWidget: 实现 IDataSink 显示接收数据
 *   - ChartWidget: 实现 IDataSink 绘制波形数据
 *   - ProtocolView: 实现 IDataSink 解析协议帧
 */
#ifndef INTERFACES_IDATASINK_H
#define INTERFACES_IDATASINK_H

#include <QByteArray>
#include <QString>

/**
 * @brief 数据接收器接口 - 数据流消费者的统一协议
 *
 * 所有需要接收数据的模块实现此接口，通过 IConnection 注册后
 * 自动接收数据流推送。
 */
class IDataSink {
public:
    virtual ~IDataSink() = default;

    /**
     * @brief 处理接收到的数据
     * @param data 收到的原始字节数据
     */
    virtual void onDataReceived(const QByteArray& data) = 0;

    /**
     * @brief 处理发送的数据（回显/记录）
     * @param data 已发送的原始字节数据
     */
    virtual void onDataSent(const QByteArray& data) = 0;

    /** @brief 清除所有已缓冲的数据 */
    virtual void clearData() = 0;

    /** @brief 获取数据接收器的名称标识 (用于日志和调试) */
    virtual QString sinkName() const = 0;
};

#endif // INTERFACES_IDATASINK_H
