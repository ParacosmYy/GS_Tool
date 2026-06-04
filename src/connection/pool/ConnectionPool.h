/**
 * @file ConnectionPool.h
 * @brief 连接池管理器，统一管理多种类型连接的生命周期和自动重连
 */
#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QTimer>

/**
 * @class ConnectionPool
 * @brief 连接池，集中管理串口/TCP/UDP等连接的创建、销毁、自动重连和活动监控
 */
class ConnectionPool : public QObject {
    Q_OBJECT
public:
    /**
     * @brief 连接池条目，描述单个连接的元信息
     */
    struct PoolEntry {
        QString id;           ///< 连接唯一标识
        QString type;         ///< 连接类型(如 serial/tcp/udp)
        QString address;      ///< 连接地址
        bool connected = false;   ///< 是否已连接
        qint64 created = 0;       ///< 创建时间戳
        qint64 lastActivity = 0;  ///< 最后活动时间戳
    };

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit ConnectionPool(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~ConnectionPool() override;

    /** @brief 创建新连接并返回其ID @param type 连接类型 @param address 连接地址 @return 连接ID */
    QString createConnection(const QString &type, const QString &address);
    /** @brief 移除指定连接 @param id 连接ID @param evicted true=池主动驱逐(溢出/超时)，false=用户主动归还 */
    void removeConnection(const QString &id, bool evicted = false);
    /** @brief 连接池中所有连接执行连接操作 */
    void connectAll();
    /** @brief 断开所有连接 */
    void disconnectAll();
    /** @brief 获取指定连接信息 @param id 连接ID @return 连接条目 */
    PoolEntry connection(const QString &id) const;
    /** @brief 获取所有连接列表 @return 连接条目列表 */
    QList<PoolEntry> allConnections() const;
    /** @brief 按类型筛选连接 @param type 连接类型 @return 匹配的连接列表 */
    QList<PoolEntry> connectionsByType(const QString &type) const;
    /** @brief 获取已连接数 @return 已连接数量 */
    int connectedCount() const;
    /** @brief 获取总连接数 @return 总数量 */
    int totalCount() const;
    /** @brief 设置最大允许连接数 @param max 最大连接数 */
    void setMaxConnections(int max);
    /** @brief 配置自动重连策略 @param enable 是否启用 @param intervalMs 重连间隔(毫秒) */
    void setAutoReconnect(bool enable, int intervalMs = 5000);
    /** @brief 刷新指定连接的活动时间 @param id 连接ID */
    void updateActivity(const QString &id);

    // ---- 统计接口 ----

    /** @brief 获取累计创建连接次数 @return 创建总次数 */
    quint64 totalCreated() const { return m_totalCreated; }

    /** @brief 获取累计移除连接次数 @return 移除总次数 */
    quint64 totalRemoved() const { return m_totalRemoved; }

    /** @brief 获取累计自动重连触发次数 @return 重连触发总次数 */
    quint64 totalReconnectAttempts() const { return m_totalReconnectAttempts; }

    /** @brief 获取累计活动刷新次数 @return 活动刷新总次数 */
    quint64 totalActivityUpdates() const { return m_totalActivityUpdates; }

    /** @brief 获取连接池满拒绝次数 @return 池满事件总次数 */
    quint64 totalPoolFullEvents() const { return m_totalPoolFullEvents; }

    /** @brief 获取累计借出尝试次数 @return 借出尝试总次数 */
    quint64 totalBorrowAttempts() const { return m_totalBorrowAttempts; }

    /** @brief 获取累计借出成功次数 @return 借出成功总次数 */
    quint64 totalBorrowSuccesses() const { return m_totalBorrowSuccesses; }

    /** @brief 获取累计归还连接次数 @return 归还总次数 */
    quint64 totalReturnCount() const { return m_totalReturnCount; }

    /** @brief 获取累计淘汰连接次数 @return 淘汰总次数 */
    quint64 totalEvictions() const { return m_totalEvictions; }

    /** @brief 获取累计借出等待超时次数 @return 超时总次数 */
    quint64 totalWaitTimeouts() const { return m_totalWaitTimeouts; }

    /** @brief 获取连接池峰值大小 @return 历史最大池大小 */
    int peakPoolSize() const { return m_peakPoolSize; }

    /** @brief 获取平均借出耗时(ms) @return 平均借出时间 */
    double avgBorrowTimeMs() const;

    /** @brief 重置所有统计计数器 */
    void resetPoolStatistics();

signals:
    /** @brief 新连接创建时发射 @param id 连接ID */
    void connectionCreated(const QString &id);
    /** @brief 连接被移除时发射 @param id 连接ID */
    void connectionRemoved(const QString &id);
    /** @brief 连接状态变化时发射 @param id 连接ID @param connected 是否已连接 */
    void connectionStateChanged(const QString &id, bool connected);
    /** @brief 连接池达到上限时发射 */
    void poolFull();

private:
    /** @brief 自动重连定时器回调 */
    void onReconnectTimer();

    QMap<QString, PoolEntry> m_pool;     ///< 连接池，ID到条目的映射
    int m_maxConnections = 16;           ///< 最大连接数
    bool m_autoReconnect = false;        ///< 是否启用自动重连
    QTimer *m_reconnectTimer = nullptr;  ///< 自动重连定时器
    int m_counter = 0;                   ///< 连接ID自增计数器

    // 统计计数器
    quint64 m_totalCreated = 0;          ///< 累计创建连接次数
    quint64 m_totalRemoved = 0;          ///< 累计移除连接次数
    quint64 m_totalReconnectAttempts = 0;///< 累计自动重连触发次数
    quint64 m_totalActivityUpdates = 0;  ///< 累计活动刷新次数
    quint64 m_totalPoolFullEvents = 0;   ///< 连接池满拒绝次数
    quint64 m_totalBorrowAttempts = 0;   ///< 累计借出尝试次数
    quint64 m_totalBorrowSuccesses = 0;  ///< 累计借出成功次数
    quint64 m_totalReturnCount = 0;      ///< 累计归还连接次数
    quint64 m_totalEvictions = 0;        ///< 累计淘汰连接次数
    quint64 m_totalWaitTimeouts = 0;     ///< 累计借出等待超时次数
    int m_peakPoolSize = 0;              ///< 连接池历史峰值大小
    qint64 m_totalBorrowTimeMs = 0;      ///< 累计借出总耗时(ms)
};

#endif // CONNECTIONPOOL_H
