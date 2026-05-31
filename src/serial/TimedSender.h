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
 *   - m_isRunning / m_interval 由 QMutex 保护，跨线程读写安全
 *   - onTimeout() 通过 QMetaObject::invokeMethod 调度到主线程
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

    /**
     * @brief 设置发送间隔（毫秒）
     * @param ms 间隔时间，必须 > 0
     *
     * 线程安全：内部加锁保护 m_interval。
     * 如果定时器正在运行，会立即应用新的间隔值。
     */
    void setInterval(int ms);

    /**
     * @brief 获取当前发送间隔（毫秒）
     * @return 间隔时间
     *
     * 线程安全：内部加锁读取 m_interval。
     */
    int interval() const;

    /**
     * @brief 设置要循环发送的单组数据
     * @param data 待发送的数据
     */
    void setData(const QByteArray& data);

    /**
     * @brief 设置多组数据队列（按顺序循环发送）
     * @param queue 数据队列列表
     */
    void setDataQueue(const QList<QByteArray>& queue);

    /**
     * @brief 启动定时发送
     *
     * 线程安全：可在定时器回调中安全调用。
     * 如果队列为空则不启动。
     */
    void start();

    /**
     * @brief 停止定时发送
     *
     * 线程安全：可在定时器回调中安全调用。
     */
    void stop();

    /**
     * @brief 查询定时发送是否正在运行
     * @return true=正在运行，false=已停止
     *
     * 线程安全：内部加锁读取 m_isRunning。
     */
    bool isRunning() const;

signals:
    /**
     * @brief 定时触发的发送信号
     * @param data 当前要发送的数据
     *
     * 此信号始终在主线程中发射，下游可安全操作 UI 和串口。
     */
    void sendData(const QByteArray& data);

private slots:
    /**
     * @brief 定时器超时处理槽
     *
     * 使用 QMetaObject::invokeMethod 将实际发送逻辑调度到主线程。
     * 在发送前检查队列有效性和连接状态。
     */
    void onTimeout();

private:
    /**
     * @brief 执行实际的发送动作（在主线程中调用）
     *
     * 从队列中取出当前数据，发射 sendData 信号，
     * 然后推进队列索引到下一个位置。
     */
    void doSend();

    QTimer m_timer;                 ///< 定时器
    QList<QByteArray> m_queue;      ///< 数据队列
    int m_queueIndex = 0;           ///< 当前队列位置

    mutable QMutex m_mutex;         ///< 保护 m_isRunning 和 m_interval 的互斥锁
    bool m_isRunning = false;       ///< 定时发送运行状态（受 m_mutex 保护）
    int m_interval = 1000;          ///< 发送间隔（毫秒，受 m_mutex 保护）
};

#endif // TIMEDSENDER_H
