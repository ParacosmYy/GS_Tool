/**
 * @file IDataSink.h
 * @brief 数据接收器接口 — 数据流水线终端的统一契约
 *
 * 数据经过解析/过滤/转换后最终流入IDataSink实现。
 * 典型实现: TerminalModel/ChartModel/RecordingController/DashboardModel。
 * 层级: L0 纯虚接口层
 */
#ifndef IDATASINK_H
#define IDATASINK_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVariantMap>
#include "shared/AppConstants.h"

/**
 * @brief 数据接收器接口
 * 协作: DataPipeline(推送) / TerminalModel/ChartModel/RecordingController(消费)
 */
class IDataSink : public QObject {
    Q_OBJECT
public:
    explicit IDataSink(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDataSink() = default;
    virtual qint64 write(const QByteArray& data, DataDirection dir = DataDirection::Rx) = 0; ///< 写入数据(RX/TX方向)
    virtual void flush() = 0;                              ///< 刷新缓冲区
    virtual void clear(bool notify = true) = 0;            ///< 清空数据
    virtual QString sinkId() const = 0;                    ///< 接收器ID
    virtual QString sinkName() const = 0;                  ///< 显示名称
    virtual bool acceptsDirection(DataDirection dir) const = 0; ///< 是否接受指定方向
    virtual bool isFull() const = 0;                       ///< 缓冲区是否已满
    virtual quint64 totalBytesReceived() const = 0;        ///< 已接收字节总数
    virtual quint64 totalPacketsReceived() const = 0;      ///< 已接收包总数
    virtual void configure(const QVariantMap& c) = 0;      ///< 配置参数
    virtual QVariantMap configuration() const = 0;         ///< 获取配置
signals:
    void dataWritten(qint64 bytes);                        ///< 数据已写入
    void dataCleared();                                    ///< 数据已清空
    void sinkStateChanged(const QString& state);           ///< 状态变化
};

#endif // IDATASINK_H
