/**
 * @file DataStatistics.h
 * @brief 数据统计面板 - 实时收发统计、滚动吞吐量、直方图和错误监控
 *
 * 职责: RX/TX累计字节/速率/峰值/持续时间/错误计数/滚动吞吐量/直方图/采样历史
 * 流程: update()→增量速率 | updateErrors()→错误显示 | onRefreshTimer()→定时刷新
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
    double lowerBound;     ///< 桶的下界(bytes/s)
    double upperBound;     ///< 桶的上界(bytes/s)
    int sampleCount = 0;   ///< 落入该桶的采样次数
    QString label;         ///< 桶的显示标签(如"0~1 KB/s")
};

/** @brief 吞吐量采样点 — 记录某一时刻的瞬时速率和包速率 */
struct ThroughputSample {
    qint64 timestampMs;    ///< 采样时间戳(相对于连接开始, ms)
    double rxBytesPerSec;  ///< RX瞬时速率(bytes/s)
    double txBytesPerSec;  ///< TX瞬时速率(bytes/s)
    double rxPacketsPerSec; ///< RX瞬时包速率(packets/s)
    double txPacketsPerSec; ///< TX瞬时包速率(packets/s)
};

/** @brief 数据统计面板 — 收发统计/滚动吞吐量/直方图/错误监控 */
class DataStatistics : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造数据统计面板 @param parent 父控件 */
    explicit DataStatistics(QWidget* parent = nullptr);

    /** @brief 更新收发累计字节数，内部计算增量得到速率 @param rxBytes RX累计字节数 @param txBytes TX累计字节数 */
    void update(quint64 rxBytes, quint64 txBytes);

    /** @brief 更新串口错误计数 @param framingErrors 帧错误次数 @param parityErrors 校验错误次数 @param overrunErrors 溢出错误次数 */
    void updateErrors(int framingErrors, int parityErrors, int overrunErrors);

    /** @brief 重置所有统计（重新连接时调用） */
    void reset();

    /** @brief 获取当前RX速率（bytes/s） */
    double rxRate() const;

    /** @brief 获取当前TX速率（bytes/s） */
    double txRate() const;

    /** @brief 更新连接健康状态显示 @param alive 连接是否存活 @param lastDataAgeMs 距上次数据的毫秒数 */
    void updateConnectionHealth(bool alive, qint64 lastDataAgeMs);

    /** @brief 生成会话统计摘要文本 @return 多行统计: 总字节/速率/峰值/持续时间/错误 */
    QString sessionSummary() const;

    /** @brief 获取RX累计总字节数 @return 接收字节总数 */
    quint64 totalRxBytes() const;

    /** @brief 获取TX累计总字节数 @return 发送字节总数 */
    quint64 totalTxBytes() const;

    // ---- 基础统计计数器 ----
    /** @brief 获取update()调用总次数 @return 更新总次数 */
    quint64 totalUpdates() const;
    /** @brief 获取历史峰值速率(RX/TX中较大者) @return 峰值速率(bytes/s) */
    double peakRate() const;
    /** @brief 获取所有update()调用传入的字节总数(RX+TX) @return 累计字节数 */
    quint64 totalBytesCounted() const;
    /** @brief 获取峰值速率更新总次数 @return 刷新次数 */
    quint64 totalPeakUpdates() const;
    /** @brief 获取updateErrors()调用总次数 @return 错误更新次数 */
    quint64 totalErrorUpdates() const;
    /** @brief 获取updateConnectionHealth()调用总次数 @return 健康检查次数 */
    quint64 totalHealthUpdates() const;
    /** @brief 获取定时器刷新总周期数 @return 刷新周期数 */
    quint64 totalRefreshCycles() const;
    /** @brief 获取update()中的速率计算总次数 @return 计算次数 */
    quint64 totalCalculations() const;
    /** @brief 获取直方图更新总次数 @return updateHistogram调用次数 */
    quint64 totalHistogramUpdates() const;
    /** @brief 获取滑动窗口重置总次数 @return 窗口淘汰次数 */
    quint64 totalSlidingWindowResets() const;
    /** @brief 获取吞吐量快照记录总次数 @return 采样点追加次数 */
    quint64 totalThroughputSnapshots() const;
    /** @brief 重置数据统计计数器(不影响面板显示) */
    void resetDataStatistics();

    // ---- 滚动吞吐量 ----
    /** @brief 获取滚动窗口RX平均速率(bytes/s) @return 最近N秒平均RX速率 */
    double rollingRxBytesPerSec() const;
    /** @brief 获取滚动窗口TX平均速率(bytes/s) @return 最近N秒平均TX速率 */
    double rollingTxBytesPerSec() const;
    /** @brief 获取滚动窗口RX包速率(packets/s) @return 最近N秒RX包速率 */
    double rollingRxPacketsPerSec() const;
    /** @brief 获取滚动窗口TX包速率(packets/s) @return 最近N秒TX包速率 */
    double rollingTxPacketsPerSec() const;
    /** @brief 获取滚动窗口大小(秒) @return 窗口秒数 */
    int rollingWindowSize() const;
    /** @brief 获取吞吐量直方图数据(RX+TX合计) @return 直方图桶列表 */
    QVector<HistogramBucket> throughputHistogram() const;
    /** @brief 获取直方图总采样次数 @return 采样总数 */
    int histogramTotalSamples() const;
    /** @brief 获取最近N个吞吐量采样点 @param maxCount 最大数量，0=全部 @return 采样点列表(时间序) */
    QVector<ThroughputSample> throughputHistory(int maxCount = 0) const;
    /** @brief 获取吞吐量采样历史最大保留条数 @return 容量 */
    int throughputHistoryCapacity() const { return kMaxThroughputSamples; }

signals:
    /** @brief 滚动吞吐量更新信号(每秒发射一次) @param rxBytesPerSec RX滚动速率 @param txBytesPerSec TX滚动速率 */
    void rollingThroughputUpdated(double rxBytesPerSec, double txBytesPerSec);

private slots:
    /** @brief 定时器回调：每秒刷新速率、峰值、持续时间、滚动窗口、直方图 */
    void onRefreshTimer();

private:
    /** @brief 初始化UI布局 */
    void setupUI();
    /** @brief 创建统计数据框架 @param label 标签文本 @param valueLabel 值标签引用(输出) @param objectName QSS objectName @param frameName 框架objectName @return 创建好的QFrame */
    QFrame* createStatsFrame(const QString& label, QLabel*& valueLabel,
                             const QString& objectName, const QString& frameName = QString());
    /** @brief 将速率格式化为人类可读字符串 @param bytesPerSec 字节速率 @return 格式化字符串 */
    QString formatRate(double bytesPerSec) const;
    /** @brief 更新滚动窗口吞吐量(每次onRefreshTimer调用) */
    void updateRollingThroughput();
    /** @brief 更新直方图数据 @param totalBytesPerSec 当前总速率 */
    void updateHistogram(double totalBytesPerSec);
    /** @brief 初始化直方图桶(定义速率分桶边界) */
    void initHistogramBuckets();

    // ---- 控件指针 ----
    QLabel* m_rxTotalLabel;        ///< RX累计字节数显示
    QLabel* m_txTotalLabel;        ///< TX累计字节数显示
    QLabel* m_rxRateLabel;         ///< RX速率显示
    QLabel* m_txRateLabel;         ///< TX速率显示
    QLabel* m_peakRateLabel;       ///< 峰值速率显示
    QLabel* m_elapsedLabel;        ///< 连接持续时间显示
    QLabel* m_errorLabel;          ///< 串口错误计数显示(默认隐藏)
    QLabel* m_healthLabel;         ///< 连接健康状态显示(默认隐藏)
    QLabel* m_avgRateLabel;        ///< 平均速率显示
    QLabel* m_rollingRateLabel;    ///< 滚动窗口平均速率显示
    QTimer m_refreshTimer;         ///< 1秒刷新定时器
    QElapsedTimer m_stopwatch;     ///< 连接持续计时器
    QElapsedTimer m_sampleTimer;   ///< 采样间隔计时器

    // ---- 数据 ----
    quint64 m_lastRxBytes = 0;     ///< 上次采样RX累计值
    quint64 m_lastTxBytes = 0;     ///< 上次采样TX累计值
    double m_rxRate = 0.0;         ///< 当前RX速率(bytes/s)
    double m_txRate = 0.0;         ///< 当前TX速率(bytes/s)
    double m_peakRxRate = 0.0;     ///< RX峰值速率(bytes/s)
    double m_peakTxRate = 0.0;     ///< TX峰值速率(bytes/s)
    double m_avgRxRate = 0.0;      ///< RX平均速率(bytes/s)
    double m_avgTxRate = 0.0;      ///< TX平均速率(bytes/s)
    quint64 m_rxPackets = 0;       ///< RX包计数
    quint64 m_txPackets = 0;       ///< TX包计数
    int m_framingErrors = 0;       ///< 帧错误累计
    int m_parityErrors = 0;        ///< 校验错误累计
    int m_overrunErrors = 0;       ///< 溢出错误累计

    // ---- 统计计数器 ----
    quint64 m_totalUpdates = 0;           ///< update()调用总次数
    quint64 m_totalBytesCounted = 0;      ///< 传入字节总数(RX+TX)
    quint64 m_totalPeakUpdates = 0;       ///< 峰值速率刷新次数
    quint64 m_totalErrorUpdates = 0;      ///< updateErrors()调用次数
    quint64 m_totalHealthUpdates = 0;     ///< updateConnectionHealth()调用次数
    quint64 m_totalRefreshCycles = 0;     ///< onRefreshTimer()周期数
    quint64 m_totalCalculations = 0;      ///< 速率计算总次数
    quint64 m_totalHistogramUpdates = 0;  ///< 直方图更新次数
    quint64 m_totalSlidingWindowResets = 0; ///< 滑动窗口淘汰次数
    quint64 m_totalThroughputSnapshots = 0; ///< 吞吐量快照次数

    // ---- 滚动吞吐量(滑动窗口) ----
    static constexpr int kRollingWindowSeconds = 5; ///< 滚动窗口大小(5秒)
    static constexpr int kMaxThroughputSamples = 300; ///< 最大采样点数(5分钟)
    struct RollingInterval { quint64 rxBytesDelta; quint64 txBytesDelta; int rxPacketDelta; int txPacketDelta; };
    std::deque<RollingInterval> m_rollingWindow; ///< 滚动窗口队列
    quint64 m_prevSecondRxBytes = 0;    ///< 上次刷新RX累计值
    quint64 m_prevSecondTxBytes = 0;    ///< 上次刷新TX累计值
    quint64 m_prevSecondRxPackets = 0;  ///< 上次刷新RX包计数
    quint64 m_prevSecondTxPackets = 0;  ///< 上次刷新TX包计数
    double m_rollingRxBytesPerSec = 0.0;   ///< 滚动RX字节速率
    double m_rollingTxBytesPerSec = 0.0;   ///< 滚动TX字节速率
    double m_rollingRxPacketsPerSec = 0.0; ///< 滚动RX包速率
    double m_rollingTxPacketsPerSec = 0.0; ///< 滚动TX包速率
    std::deque<ThroughputSample> m_throughputHistory; ///< 采样历史队列
    QVector<HistogramBucket> m_histogramBuckets; ///< 直方图桶列表
    int m_histogramTotalSamples = 0; ///< 直方图总采样数
};

#endif // DATASTATISTICS_H
