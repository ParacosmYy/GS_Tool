/**
 * @file DataStatistics.h
 * @brief 数据统计面板 - 实时收发统计、滚动吞吐量、直方图和错误监控
 *
 * 职责:
 *   1. 显示RX/TX累计字节数和实时传输速率
 *   2. 记录并显示峰值速率（最大吞吐量）
 *   3. 显示连接持续时间（HH:MM:SS）
 *   4. 显示串口通信错误计数（帧错误/校验错误/溢出）
 *   5. 滚动吞吐量计算（基于滑动窗口的bytes/sec和packets/sec）
 *   6. 吞吐量直方图数据（分桶统计速率分布）
 *   7. 提供格式化的字节/速率字符串
 *
 * 数据更新流程:
 *   外部调用 update(rxBytes, txBytes) → 计算增量速率 → 更新累计显示
 *   外部调用 updateErrors(framing, parity, overrun) → 更新错误计数显示
 *   定时器每秒触发 onRefreshTimer() → 刷新速率/峰值/持续时间/滚动窗口/直方图
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

/**
 * @brief 吞吐量直方图桶定义
 *
 * 将速率范围划分为若干桶，统计每个速率区间的采样次数。
 * 用于可视化速率分布，识别传输模式。
 */
struct HistogramBucket {
    double lowerBound;     ///< 桶的下界(bytes/s)
    double upperBound;     ///< 桶的上界(bytes/s)
    int sampleCount = 0;   ///< 落入该桶的采样次数
    QString label;         ///< 桶的显示标签(如"0~1 KB/s")
};

/**
 * @brief 吞吐量采样点 - 记录某一时刻的瞬时速率
 */
struct ThroughputSample {
    qint64 timestampMs;    ///< 采样时间戳(相对于连接开始, ms)
    double rxBytesPerSec;  ///< RX瞬时速率(bytes/s)
    double txBytesPerSec;  ///< TX瞬时速率(bytes/s)
    double rxPacketsPerSec; ///< RX瞬时包速率(packets/s)
    double txPacketsPerSec; ///< TX瞬时包速率(packets/s)
};

/**
 * @brief 数据统计面板 - 实时收发统计、滚动吞吐量与错误监控
 *
 * 增强功能:
 *   - 滚动窗口吞吐量: 基于最近N秒的滑动窗口计算平均bytes/sec和packets/sec
 *   - 直方图数据: 将速率采样分桶统计，用于可视化速率分布
 *   - 吞吐量采样历史: 记录最近N个采样点，供外部绘制速率曲线
 */
class DataStatistics : public QWidget {
    Q_OBJECT
public:
    explicit DataStatistics(QWidget* parent = nullptr);

    /** @brief 更新收发累计字节数，内部计算增量得到速率
     * @param rxBytes RX累计字节数
     * @param txBytes TX累计字节数
     */
    void update(quint64 rxBytes, quint64 txBytes);

    /** @brief 更新串口错误计数
     * @param framingErrors 帧错误次数
     * @param parityErrors 校验错误次数
     * @param overrunErrors 溢出错误次数
     */
    void updateErrors(int framingErrors, int parityErrors, int overrunErrors);

    /** @brief 重置所有统计（重新连接时调用） */
    void reset();

    /** @brief 获取当前RX速率（bytes/s） */
    double rxRate() const;

    /** @brief 获取当前TX速率（bytes/s） */
    double txRate() const;

    /** @brief 更新连接健康状态显示
     *  @param alive 连接是否存活
     *  @param lastDataAgeMs 距上次数据的毫秒数
     */
    void updateConnectionHealth(bool alive, qint64 lastDataAgeMs);

    /** @brief 生成会话统计摘要文本(用于导出/复制/Toast)
     *  @return 格式化的多行统计摘要: 总字节/速率/峰值/持续时间/错误
     */
    QString sessionSummary() const;

    /** @brief 获取RX累计总字节数 */
    quint64 totalRxBytes() const;

    /** @brief 获取TX累计总字节数 */
    quint64 totalTxBytes() const;

    // ---- 基础统计计数器 ----

    /** @brief 获取update()调用总次数 */
    quint64 totalUpdates() const;
    /** @brief 获取历史峰值速率(RX/TX中较大者) */
    double peakRate() const;
    /** @brief 获取所有update()调用传入的字节总数(RX+TX) */
    quint64 totalBytesCounted() const;
    /** @brief 获取峰值速率更新总次数 */
    quint64 totalPeakUpdates() const;
    /** @brief 获取updateErrors()调用总次数 */
    quint64 totalErrorUpdates() const { return m_totalErrorUpdates; }
    /** @brief 获取updateConnectionHealth()调用总次数 */
    quint64 totalHealthUpdates() const { return m_totalHealthUpdates; }
    /** @brief 获取定时器刷新总周期数 */
    quint64 totalRefreshCycles() const { return m_totalRefreshCycles; }
    /** @brief 获取update()中的速率计算总次数 */
    quint64 totalCalculations() const { return m_totalCalculations; }
    /** @brief 重置数据统计计数器(不影响面板显示) */
    void resetDataStatistics();

    // ---- 滚动吞吐量 ----

    /** @brief 获取滚动窗口RX平均速率(bytes/s) @return 最近kRollingWindowSeconds秒的平均RX速率 */
    double rollingRxBytesPerSec() const { return m_rollingRxBytesPerSec; }

    /** @brief 获取滚动窗口TX平均速率(bytes/s) @return 最近kRollingWindowSeconds秒的平均TX速率 */
    double rollingTxBytesPerSec() const { return m_rollingTxBytesPerSec; }

    /** @brief 获取滚动窗口RX包速率(packets/s) @return 最近kRollingWindowSeconds秒的RX包速率 */
    double rollingRxPacketsPerSec() const { return m_rollingRxPacketsPerSec; }

    /** @brief 获取滚动窗口TX包速率(packets/s) @return 最近kRollingWindowSeconds秒的TX包速率 */
    double rollingTxPacketsPerSec() const { return m_rollingTxPacketsPerSec; }

    /** @brief 获取滚动窗口大小(秒) @return 窗口秒数 */
    int rollingWindowSize() const { return kRollingWindowSeconds; }

    // ---- 直方图 ----

    /** @brief 获取吞吐量直方图数据(RX+TX合计) @return 直方图桶列表 */
    QVector<HistogramBucket> throughputHistogram() const { return m_histogramBuckets; }

    /** @brief 获取直方图总采样次数 */
    int histogramTotalSamples() const { return m_histogramTotalSamples; }

    // ---- 吞吐量采样历史 ----

    /** @brief 获取最近N个吞吐量采样点(用于绘制速率曲线) @param maxCount 最大返回数量，0=全部 @return 采样点列表(按时间顺序) */
    QVector<ThroughputSample> throughputHistory(int maxCount = 0) const;

    /** @brief 获取吞吐量采样历史最大保留条数 */
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

    /** @brief 创建统计数据框架
     * @param label 标签文本
     * @param valueLabel 用于显示值的QLabel指针(输出参数)
     * @param objectName valueLabel的QSS objectName
     * @param frameName QFrame的QSS objectName
     * @return 创建好的QFrame
     */
    QFrame* createStatsFrame(const QString& label, QLabel*& valueLabel,
                             const QString& objectName, const QString& frameName = QString());

    /** @brief 将速率格式化为带"/s"后缀的人类可读字符串 */
    QString formatRate(double bytesPerSec) const;

    /** @brief 更新滚动窗口吞吐量(每次onRefreshTimer调用) */
    void updateRollingThroughput();

    /** @brief 更新直方图数据(将当前速率采样分桶) @param totalBytesPerSec 当前总速率 */
    void updateHistogram(double totalBytesPerSec);

    /** @brief 初始化直方图桶(定义速率分桶边界) */
    void initHistogramBuckets();

    // ---- 控件指针 ----
    QLabel* m_rxTotalLabel;        ///< RX累计字节数显示
    QLabel* m_txTotalLabel;        ///< TX累计字节数显示
    QLabel* m_rxRateLabel;         ///< RX速率显示
    QLabel* m_txRateLabel;         ///< TX速率显示
    QLabel* m_peakRateLabel;       ///< 峰值速率显示(取RX/TX中较大者)
    QLabel* m_elapsedLabel;        ///< 连接持续时间显示
    QLabel* m_errorLabel;          ///< 串口错误计数显示(默认隐藏)
    QLabel* m_healthLabel;         ///< 连接健康状态显示(空闲提示,默认隐藏)
    QLabel* m_avgRateLabel;        ///< 平均速率显示(RX+TX合计)
    QLabel* m_rollingRateLabel;    ///< 滚动窗口平均速率显示

    // ---- 定时器 ----
    QTimer m_refreshTimer;         ///< 1秒刷新定时器，用于速率采样和UI更新
    QElapsedTimer m_stopwatch;     ///< 连接持续计时器
    QElapsedTimer m_sampleTimer;   ///< 采样间隔计时器，用于计算精确速率

    // ---- 基础统计数据 ----
    quint64 m_lastRxBytes = 0;     ///< 上一次采样时的RX累计值
    quint64 m_lastTxBytes = 0;     ///< 上一次采样时的TX累计值
    double m_rxRate = 0.0;         ///< 当前RX速率（bytes/s）
    double m_txRate = 0.0;         ///< 当前TX速率（bytes/s）
    double m_peakRxRate = 0.0;     ///< RX峰值速率（bytes/s）
    double m_peakTxRate = 0.0;     ///< TX峰值速率（bytes/s）
    double m_avgRxRate = 0.0;      ///< RX平均速率（bytes/s，基于整个会话）
    double m_avgTxRate = 0.0;      ///< TX平均速率（bytes/s，基于整个会话）
    quint64 m_rxPackets = 0;       ///< RX数据包计数（每次update调用+1）
    quint64 m_txPackets = 0;       ///< TX数据包计数（每次update调用+1）

    // ---- 错误计数 ----
    int m_framingErrors = 0;       ///< 帧错误累计
    int m_parityErrors = 0;        ///< 校验错误累计
    int m_overrunErrors = 0;       ///< 溢出错误累计

    // ---- 统计计数器 ----
    quint64 m_totalUpdates = 0;    ///< update()调用总次数
    quint64 m_totalBytesCounted = 0; ///< 所有update()传入的字节总数(RX+TX)
    quint64 m_totalPeakUpdates = 0;  ///< 峰值速率更新总次数
    quint64 m_totalErrorUpdates = 0; ///< updateErrors()调用总次数
    quint64 m_totalHealthUpdates = 0; ///< updateConnectionHealth()调用总次数
    quint64 m_totalRefreshCycles = 0; ///< onRefreshTimer()定时器刷新总周期数
    quint64 m_totalCalculations = 0;  ///< update()中的速率计算总次数

    // ---- 滚动吞吐量(滑动窗口) ----
    static constexpr int kRollingWindowSeconds = 5; ///< 滚动窗口大小(5秒)
    static constexpr int kMaxThroughputSamples = 300; ///< 最多保留300个采样点(5分钟)

    /**
     * @brief 滚动窗口增量记录
     * 记录每秒的字节增量和包增量，用于计算滚动速率。
     */
    struct RollingInterval {
        quint64 rxBytesDelta;   ///< 该秒内RX字节增量
        quint64 txBytesDelta;   ///< 该秒内TX字节增量
        int rxPacketDelta;      ///< 该秒内RX包增量
        int txPacketDelta;      ///< 该秒内TX包增量
    };

    std::deque<RollingInterval> m_rollingWindow; ///< 滚动窗口队列(最多kRollingWindowSeconds个元素)
    quint64 m_prevSecondRxBytes = 0;  ///< 上一次onRefreshTimer时的RX累计值
    quint64 m_prevSecondTxBytes = 0;  ///< 上一次onRefreshTimer时的TX累计值
    quint64 m_prevSecondRxPackets = 0;///< 上一次onRefreshTimer时的RX包计数
    quint64 m_prevSecondTxPackets = 0;///< 上一次onRefreshTimer时的TX包计数
    double m_rollingRxBytesPerSec = 0.0;   ///< 滚动窗口RX平均字节速率
    double m_rollingTxBytesPerSec = 0.0;   ///< 滚动窗口TX平均字节速率
    double m_rollingRxPacketsPerSec = 0.0; ///< 滚动窗口RX平均包速率
    double m_rollingTxPacketsPerSec = 0.0; ///< 滚动窗口TX平均包速率

    // ---- 吞吐量采样历史 ----
    std::deque<ThroughputSample> m_throughputHistory; ///< 采样历史队列

    // ---- 直方图 ----
    QVector<HistogramBucket> m_histogramBuckets; ///< 直方图桶列表
    int m_histogramTotalSamples = 0;             ///< 直方图总采样次数
};

#endif // DATASTATISTICS_H
