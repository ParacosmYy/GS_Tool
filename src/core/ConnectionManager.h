#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QList>
#include "connection/IConnection.h"

// 连接管理器 - 统一管理所有连接实例(串口/TCP/UDP)
// 提供创建、删除、查询连接的接口
class ConnectionManager : public QObject {
    Q_OBJECT

public:
    explicit ConnectionManager(QObject* parent = nullptr);
    ~ConnectionManager() override;

    // 创建一个新的串口连接，返回连接对象指针
    IConnection* createSerialConnection();

    // 删除一个连接
    void removeConnection(IConnection* conn);

    // 获取所有连接
    QList<IConnection*> connections() const;

    // 获取指定连接
    IConnection* connection(int index) const;

    // 获取连接数量
    int count() const;

signals:
    // 连接列表变化
    void connectionAdded(IConnection* conn);
    void connectionRemoved(IConnection* conn);

private:
    QList<IConnection*> m_connections;  // 连接列表
};

#endif // CONNECTIONMANAGER_H
