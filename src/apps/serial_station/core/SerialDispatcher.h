#ifndef SERIAL_DISPATCHER_H
#define SERIAL_DISPATCHER_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

#include <memory>

#include "apps/serial_station/protocols/ISerialProtocol.h"
#include "apps/serial_station/protocols/SerialProtocolEvent.h"

namespace serial_station {

/**
 * @brief 串口原始字节分发器。
 *
 * 只负责把 QByteArray 交给当前协议 feed，并记录轻量状态；不生成 UI 文案。
 */
class SerialDispatcher {
public:
    /**
     * @brief 最近一次 feed 的状态。
     */
    enum class FeedStatus {
        EmptyInput,
        MissingProtocol,
        Buffered,
        EventsReady
    };

    /**
     * @brief 最近一次 feed 的摘要。
     */
    struct FeedSummary {
        FeedStatus status = FeedStatus::EmptyInput;
        int inputBytes = 0;
        int eventCount = 0;
        QString protocolName;
    };

    /**
     * @brief 设置当前协议实例。
     * @param protocol 协议实例，允许为空
     */
    void setProtocol(std::unique_ptr<ISerialProtocol> protocol);

    /**
     * @brief 当前是否存在可用协议。
     * @return true 表示已设置协议
     */
    bool hasProtocol() const;

    /**
     * @brief 当前协议名称。
     * @return 协议名称；没有协议时返回空字符串
     */
    QString protocolName() const;

    /**
     * @brief 分发原始接收 bytes。
     * @param bytes 原始接收数据
     * @return 协议产生的结构化事件
     */
    QVector<SerialProtocolEvent> feed(const QByteArray& bytes);

    /**
     * @brief 重置当前协议内部状态。
     */
    void reset();

    /**
     * @brief 最近一次 feed 的摘要。
     * @return feed 状态摘要
     */
    FeedSummary lastFeedSummary() const;

private:
    void updateSummary(FeedStatus status, int inputBytes, int eventCount);

    std::unique_ptr<ISerialProtocol> m_protocol;
    FeedSummary m_lastSummary;
};

} // namespace serial_station

#endif // SERIAL_DISPATCHER_H
