/**
 * @file DataStatistics.h
 * @brief 数据统计面板 - 实时收发统计、滚动吞吐量、直方图和错误监控
 *
 * 职责: RX/TX累计字节/速率/峰值/持续时间/错误计数/滚动吞吐量/直方图/采样历史
 */
#ifndef DATASTATISTICS_H
#define DATASTATISTICS_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPair>
#include <deque>

/** @brief 吞吐量直方图桶定义 — 分桶统计速率分布 */
struct HistogramBucket {
    double lowerBound, upperBound; int sampleCount = 0; QString label;
};

/** @brief 吞吐量采样点 — 瞬时速率和包速率 */
struct ThroughputSample {
    qint64 timestampMs;
    double rxBytesPerSec, txBytesPerSec, rxPacketsPerSec, txPacketsPerSec;
};

/// @brief 数据统计面板 — 收发统计/滚动吞吐量/直方图/错误监控
class DataStatistics : public QWidget {
    Q_OBJECT
public:
    explicit DataStatistics(QWidget* parent = nullptr);
    void update(quint64 rxBytes, quint64 txBytes);  ///< 更新收发累计字节数
    void updateErrors(int framingErrors, int parityErrors, int overrunErrors); ///< 更新错误计数
    void reset();                                    ///< 重置所有统计
    double rxRate() const;                           ///< 当前RX速率(bytes/s)
    double txRate() const;                           ///< 当前TX速率(bytes/s)
    void updateConnectionHealth(bool alive, qint64 lastDataAgeMs); ///< 连接健康状态
    QString sessionSummary() const;                  ///< 会话统计摘要文本
    quint64 totalRxBytes() const;
    quint64 totalTxBytes() const;
    quint64 totalRxPackets() const { return m_rxPackets; }
    quint64 totalTxPackets() const { return m_txPackets; }

    // ---- 基础统计计数器 ----
    quint64 totalUpdates() const;
    double peakRate() const;
    quint64 totalBytesCounted() const;
    quint64 totalPeakUpdates() const;
    quint64 totalErrorUpdates() const;
    quint64 totalHealthUpdates() const;
    quint64 totalRefreshCycles() const;
    quint64 totalCalculations() const;
    quint64 totalHistogramUpdates() const;
    quint64 totalSlidingWindowResets() const;
    quint64 totalThroughputSnapshots() const;
    quint64 totalResets() const;
    quint64 totalFormatChanges() const;
    void resetDataStatistics();

    // ---- 滚动吞吐量 ----
    double rollingRxBytesPerSec() const;
    double rollingTxBytesPerSec() const;
    double rollingRxPacketsPerSec() const;
    double rollingTxPacketsPerSec() const;
    int rollingWindowSize() const;
    QVector<HistogramBucket> throughputHistogram() const;
    int histogramTotalSamples() const;
    QVector<ThroughputSample> throughputHistory(int maxCount = 0) const;
    int throughputHistoryCapacity() const { return kMaxThroughputSamples; }

signals:
    void rollingThroughputUpdated(double rxBytesPerSec, double txBytesPerSec);

private slots:
    void onRefreshTimer();

private:
    void setupUI();
    QFrame* createStatsFrame(const QString& label, QLabel*& valueLabel, const QString& objectName, const QString& frameName = QString());
    QString formatRate(double bytesPerSec) const;
    void updateRollingThroughput();
    void updateHistogram(double totalBytesPerSec);
    void initHistogramBuckets();

    QLabel* m_rxTotalLabel; QLabel* m_txTotalLabel; QLabel* m_rxRateLabel; QLabel* m_txRateLabel;
    QLabel* m_peakRateLabel; QLabel* m_elapsedLabel; QLabel* m_errorLabel;
    QLabel* m_healthLabel; QLabel* m_avgRateLabel; QLabel* m_rollingRateLabel;
    QTimer m_refreshTimer; QElapsedTimer m_stopwatch; QElapsedTimer m_sampleTimer;
    quint64 m_lastRxBytes = 0, m_lastTxBytes = 0;
    double m_rxRate = 0.0, m_txRate = 0.0, m_peakRxRate = 0.0, m_peakTxRate = 0.0;
    double m_avgRxRate = 0.0, m_avgTxRate = 0.0;
    quint64 m_rxPackets = 0, m_txPackets = 0;
    int m_framingErrors = 0, m_parityErrors = 0, m_overrunErrors = 0;
    quint64 m_totalUpdates = 0, m_totalBytesCounted = 0, m_totalPeakUpdates = 0;
    quint64 m_totalErrorUpdates = 0, m_totalHealthUpdates = 0, m_totalRefreshCycles = 0;
    quint64 m_totalCalculations = 0, m_totalHistogramUpdates = 0, m_totalSlidingWindowResets = 0;
    quint64 m_totalThroughputSnapshots = 0, m_totalResets = 0, m_totalFormatChanges = 0;
    static constexpr int kRollingWindowSeconds = 5;
    static constexpr int kMaxThroughputSamples = 300;
    struct RollingInterval { quint64 rxBytesDelta, txBytesDelta; int rxPacketDelta, txPacketDelta; };
    std::deque<RollingInterval> m_rollingWindow;
    quint64 m_prevSecondRxBytes = 0, m_prevSecondTxBytes = 0;
    quint64 m_prevSecondRxPackets = 0, m_prevSecondTxPackets = 0;
    double m_rollingRxBytesPerSec = 0.0, m_rollingTxBytesPerSec = 0.0;
    double m_rollingRxPacketsPerSec = 0.0, m_rollingTxPacketsPerSec = 0.0;
    std::deque<ThroughputSample> m_throughputHistory;
    QVector<HistogramBucket> m_histogramBuckets;
    int m_histogramTotalSamples = 0;
};

#endif // DATASTATISTICS_H
