/**
 * @file TimedSender.h
 * @brief 定时发送器 - 按设定间隔循环发送数据
 *
 * 职责:
 *   1. 支持单组数据或队列数据按顺序循环发送
 *   2. 通过 QMutex 保护 m_isRunning / m_interval 等状态的并发安全
 *   3. 定时器回调中使用 QMetaObject::invokeMethod 确保在主线程执行发送
 *   4. 在发送前检查连接有效性（通过 canSend 信号询问外部是否有活跃连接）
 *
 * 线程安全设计:
 *   - start() / stop() 可在定时器回调中安全调用（加锁保护）
 *   - m_isRunning / m_interval / m_queue / m_queueIndex 由 QMutex 保护，跨线程读写安全
 *   - onTimeout() 通过 QMetaObject::invokeMethod 调度到主线程
 *   - doSend() 在锁内拷贝数据后释放锁再 emit，避免信号回调死锁
 *
 * 协作关系:
 *   - SendController: 创建并管理 TimedSender，连接 sendData 信号
 *   - ConnectionController: 接收 sendData 信号并写入串口/网络
 */

#ifndef TIMEDSENDER_H
#define TIMEDSENDER_H

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>

/**
 * @brief 定时发送器 - 按设定间隔循环发送数据
 *
 * 支持发送队列：可以配置多组数据按顺序循环发送。
 * 内部使用 QMutex 保护关键状态变量，确保在定时器回调和外部调用之间
 * 的并发访问安全。定时器触发时通过 QMetaObject::invokeMethod
 * 将发送动作调度到主线程执行。
 */
class TimedSender : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造定时发送器
     * @param parent 父对象（通常为 SendController）
     */
    explicit TimedSender(QObject* parent = nullptr);

    /** @brief 设置发送间隔（毫秒），线程安全 */
    void setInterval(int ms);

    /** @brief 获取当前发送间隔（毫秒），线程安全 */
    int interval() const;

    /** @brief 设置要循环发送的单组数据 */
    void setData(const QByteArray& data);

    /** @brief 设置多组数据队列（按顺序循环发送） */
    void setDataQueue(const QList<QByteArray>& queue);

    /** @brief 启动定时发送，队列为空则不启动 */
    void start();

    /** @brief 停止定时发送 */
    void stop();

    /** @brief 查询定时发送是否正在运行，线程安全 */
    bool isRunning() const;

    /** @brief 获取已发送次数 */
    int sendCount() const;

    /** @brief 获取累计发送的总字节数 */
    quint64 totalBytesSent() const;

    /** @brief 获取定时发送调度次数（每次 start() 调用 +1） */
    quint64 scheduleCount() const;

    /** @brief 获取定时器触发发送的总次数 */
    quint64 totalTimedSends() const;

    /** @brief 获取定时发送累计字节数 @return 定时发送写入的总字节数 */
    quint64 totalTimedBytesSent() const;

    /** @brief 获取定时发送累计错误次数 @return 发送失败/队列为空等错误次数 */
    quint64 totalTimedErrors() const;

    /** @brief 重置所有统计计数器（sendCount/totalBytesSent/scheduleCount/totalTimedSends/totalTimedBytesSent/totalTimedErrors） */
    void resetStatistics();

signals:
    /**
     * @brief 定时触发的发送信号
     * @param data 当前要发送的数据
     *
     * 此信号始终在主线程中发射，下游可安全操作 UI 和串口。
     */
    void sendData(const QByteArray& data);

private slots:
    /** @brief 定时器超时处理槽，调度到主线程执行 */
    void onTimeout();

private:
    /** @brief 执行实际发送动作（在主线程中调用） */
    void doSend();

    QTimer m_timer;                 ///< 定时器
    QList<QByteArray> m_queue;      ///< 数据队列
    int m_queueIndex = 0;           ///< 当前队列位置

    mutable QMutex m_mutex;         ///< 保护所有状态的互斥锁
    bool m_isRunning = false;       ///< 定时发送运行状态（受 m_mutex 保护）
    int m_interval = 1000;          ///< 发送间隔（毫秒，受 m_mutex 保护）
    int m_sendCount = 0;            ///< 已发送次数计数（受 m_mutex 保护）
    quint64 m_totalBytesSent = 0;   ///< 累计发送总字节数（受 m_mutex 保护）
    quint64 m_scheduleCount = 0;    ///< 定时发送调度次数（受 m_mutex 保护）
    quint64 m_totalTimedSends = 0;  ///< 定时器触发发送的总次数（受 m_mutex 保护）
    quint64 m_totalTimedBytesSent = 0; ///< 定时发送累计字节数（受 m_mutex 保护）
    quint64 m_totalTimedErrors = 0; ///< 定时发送累计错误次数（受 m_mutex 保护）
};

#endif // TIMEDSENDER_H
