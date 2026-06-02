/**
 * @file CanConnection.h
 * @brief CAN/CAN-FD连接实现 — 适配器模式，封装CAN总线到IConnection接口
 *
 * 职责: CAN/CAN-FD总线连接管理、帧收发、波特率配置，
 * 通过IConnection统一接口供上层使用。
 */
#ifndef CANCONNECTION_H
#define CANCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QByteArray>
#include <QString>

/**
 * @brief CAN/CAN-FD总线连接实现
 *
 * 封装CAN总线底层通信，实现IConnection统一接口。
 * 支持经典CAN和CAN-FD模式，可配置波特率。
 */
class CanConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 构造CAN连接
     * @param parent 父对象
     */
    explicit CanConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动关闭连接 */
    ~CanConnection() override;

    // ---- IConnection 接口实现 ----

    /** @brief 返回连接类型 */
    ConnectionType type() const override;

    /** @brief 返回CAN适配器名称 */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开CAN连接 */
    bool open() override;

    /** @brief 关闭CAN连接 */
    void close() override;

    /**
     * @brief 写入(发送)CAN帧数据
     * @param data 待发送数据(封装为CAN帧)
     * @return 实际写入字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过参数映射配置CAN连接
     * @param params 支持的key: bitrate, canFd, adapter
     */
    void configure(const QVariantMap& params) override;

    // ---- CAN专用接口 ----

    /**
     * @brief 设置CAN总线波特率
     * @param bitrate 波特率(如 500000)
     */
    void setBitrate(int bitrate);

    /**
     * @brief 启用/禁用CAN-FD模式
     * @param enabled true=启用CAN-FD
     */
    void setCanFdEnabled(bool enabled);

    /**
     * @brief 发送CAN帧
     * @param id 帧ID
     * @param data 帧数据
     * @param extended true=扩展帧(29位ID)
     * @return true=发送成功
     */
    bool sendFrame(int id, const QByteArray& data, bool extended = false);

signals:
    /**
     * @brief 收到CAN帧时发出
     * @param id 帧ID
     * @param data 帧数据
     * @param extended true=扩展帧
     * @param rtr true=远程帧
     */
    void frameReceived(int id, const QByteArray& data, bool extended, bool rtr);

private:
    /** @brief CAN总线波特率，默认500kbps */
    int m_bitrate = 500000;

    /** @brief CAN-FD模式开关 */
    bool m_canFdEnabled = false;

    /** @brief 适配器名称 */
    QString m_adapterName;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;
};

#endif // CANCONNECTION_H
