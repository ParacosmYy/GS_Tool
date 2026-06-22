/**
 * @file IDataSink.h
 * @brief 数据接收接口
 */
#ifndef IDATA_SINK_H
#define IDATA_SINK_H
#include <QByteArray>
#include <QString>
class IDataSink {
public:
    virtual ~IDataSink() = default;
    virtual void onReceived(const QByteArray& data) = 0;
    virtual void clearData() = 0;
    virtual QString sinkName() const = 0;
};
#endif
