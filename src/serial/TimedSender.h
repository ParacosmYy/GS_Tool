#ifndef TIMEDSENDER_H
#define TIMEDSENDER_H

#include <QObject>
#include <QTimer>
#include <QByteArray>

// 定时发送器 - 按设定间隔循环发送数据
// 支持发送队列：可以配置多组数据按顺序循环发送
class TimedSender : public QObject {
    Q_OBJECT

public:
    explicit TimedSender(QObject* parent = nullptr);

    // 设置发送间隔(毫秒)
    void setInterval(int ms);

    // 获取发送间隔
    int interval() const;

    // 设置要循环发送的数据
    void setData(const QByteArray& data);

    // 设置多组数据队列
    void setDataQueue(const QList<QByteArray>& queue);

    // 启动定时发送
    void start();

    // 停止定时发送
    void stop();

    // 是否正在运行
    bool isRunning() const;

signals:
    // 定时触发的发送信号
    void sendData(const QByteArray& data);

private slots:
    void onTimeout();

private:
    QTimer m_timer;                 // 定时器
    QList<QByteArray> m_queue;      // 数据队列
    int m_queueIndex = 0;           // 当前队列位置
};

#endif // TIMEDSENDER_H
