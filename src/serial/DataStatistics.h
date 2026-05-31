#ifndef DATASTATISTICS_H
#define DATASTATISTICS_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

// 数据统计面板 - 实时显示收发字节数和速率
// 职责：
//   1. 显示RX/TX累计字节数
//   2. 每秒采样计算实时传输速率
//   3. 显示连接持续时间（HH:MM:SS）
//   4. 提供格式化的字节/速率字符串
class DataStatistics : public QWidget {
    Q_OBJECT
public:
    explicit DataStatistics(QWidget* parent = nullptr);

    // 更新统计数据（每次收发数据时调用）
    // 传入的是累计值，内部计算增量来得到速率
    void update(quint64 rxBytes, quint64 txBytes);

    // 重置所有统计（重新连接时调用）
    void reset();

    // 获取当前速率（bytes/s）
    double rxRate() const;
    double txRate() const;

private slots:
    // 定时器回调：每秒刷新速率和持续时间
    void onRefreshTimer();

private:
    void setupUI();

    // 将字节数格式化为人类可读字符串
    // <1024 显示 B, <1M 显示 KB, <1G 显示 MB, 否则显示 GB
    QString formatBytes(quint64 bytes) const;

    // 将速率格式化为带 "/s" 后缀的人类可读字符串
    QString formatRate(double bytesPerSec) const;

    QLabel* m_rxTotalLabel;        // RX累计字节数显示
    QLabel* m_txTotalLabel;        // TX累计字节数显示
    QLabel* m_rxRateLabel;         // RX速率显示
    QLabel* m_txRateLabel;         // TX速率显示
    QLabel* m_elapsedLabel;        // 连接持续时间显示

    QTimer m_refreshTimer;         // 1秒刷新定时器，用于速率采样和UI更新
    QElapsedTimer m_stopwatch;     // 连接持续计时器
    QElapsedTimer m_sampleTimer;   // 采样间隔计时器，用于计算精确速率

    quint64 m_lastRxBytes = 0;     // 上一次采样时的RX累计值
    quint64 m_lastTxBytes = 0;     // 上一次采样时的TX累计值
    double m_rxRate = 0.0;         // 当前RX速率（bytes/s）
    double m_txRate = 0.0;         // 当前TX速率（bytes/s）
};

#endif // DATASTATISTICS_H
