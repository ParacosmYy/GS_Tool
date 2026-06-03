/**
 * @file DataStatistics.h
 * @brief 数据统计面板 - 实时显示收发字节数、速率、峰值和串口错误计数
 *
 * 职责:
 *   1. 显示RX/TX累计字节数和实时传输速率
 *   2. 记录并显示峰值速率（最大吞吐量）
 *   3. 显示连接持续时间（HH:MM:SS）
 *   4. 显示串口通信错误计数（帧错误/校验错误/溢出）
 *   5. 提供格式化的字节/速率字符串
 */

#ifndef DATASTATISTICS_H
#define DATASTATISTICS_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

/**
 * @brief 数据统计面板 - 实时收发统计与错误监控
 *
 * 数据更新流程:
 *   外部调用 update(rxBytes, txBytes) → 计算增量速率 → 更新累计显示
 *   外部调用 updateErrors(framing, parity, overrun) → 更新错误计数显示
 *   定时器每秒触发 onRefreshTimer() → 刷新速率/峰值/持续时间
 *
 * 峰值速率在 onRefreshTimer() 中更新，取历史最大值。
 * 错误计数面板默认隐藏，有错误时自动显示。
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
     *
     *  当 lastDataAgeMs > 10000 时显示"空闲 Xs"提示，数据正常流动时隐藏
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

    // ── 统计计数器 Getter ──

    /** @brief 获取update()调用总次数 @return 累计更新次数 */
    quint64 totalUpdates() const;

    /** @brief 获取历史峰值速率(RX/TX中较大者) @return 峰值速率(bytes/s) */
    double peakRate() const;

    /** @brief 获取所有update()调用传入的字节总数(RX+TX) @return 累计字节数 */
    quint64 totalBytesCounted() const;

    /** @brief 获取峰值速率更新总次数 @return 峰值更新次数 */
    quint64 totalPeakUpdates() const;

    /** @brief 获取updateErrors()调用总次数 @return 错误更新调用次数 */
    quint64 totalErrorUpdates() const { return m_totalErrorUpdates; }

    /** @brief 获取updateConnectionHealth()调用总次数 @return 健康检查调用次数 */
    quint64 totalHealthUpdates() const { return m_totalHealthUpdates; }

    /** @brief 获取定时器刷新总周期数 @return onRefreshTimer()调用总次数 */
    quint64 totalRefreshCycles() const { return m_totalRefreshCycles; }

    /** @brief 获取update()中的速率计算总次数 @return 累计计算次数 */
    quint64 totalCalculations() const { return m_totalCalculations; }

    /** @brief 重置数据统计计数器(不影响面板显示) */
    void resetDataStatistics();

private slots:
    /** @brief 定时器回调：每秒刷新速率、峰值和持续时间 */
    void onRefreshTimer();

private:
    /** @brief 初始化UI布局: 创建所有统计框架(RX/TX/速率/峰值/时间/错误/健康)并排列 */
    void setupUI();

    /** @brief 创建统计数据框架(RX/TX/峰值/时间 统一格式)
     * @param label 标签文本(如"接收:")
     * @param valueLabel 用于显示值的QLabel指针(输出参数)
     * @param objectName valueLabel的QSS objectName
     * @param frameName QFrame的QSS objectName(如statsRxFrame)
     * @return 创建好的QFrame
     */
    QFrame* createStatsFrame(const QString& label, QLabel*& valueLabel,
                             const QString& objectName, const QString& frameName = QString());

    /** @brief 将速率格式化为带"/s"后缀的人类可读字符串 */
    QString formatRate(double bytesPerSec) const;

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

    // ---- 定时器 ----
    QTimer m_refreshTimer;         ///< 1秒刷新定时器，用于速率采样和UI更新
    QElapsedTimer m_stopwatch;     ///< 连接持续计时器
    QElapsedTimer m_sampleTimer;   ///< 采样间隔计时器，用于计算精确速率

    // ---- 统计数据 ----
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

    // ── 统计计数器 ──
    quint64 m_totalUpdates = 0;    ///< update()调用总次数
    quint64 m_totalBytesCounted = 0; ///< 所有update()传入的字节总数(RX+TX)
    quint64 m_totalPeakUpdates = 0;  ///< 峰值速率更新总次数
    quint64 m_totalErrorUpdates = 0; ///< updateErrors()调用总次数
    quint64 m_totalHealthUpdates = 0; ///< updateConnectionHealth()调用总次数
    quint64 m_totalRefreshCycles = 0; ///< onRefreshTimer()定时器刷新总周期数
    quint64 m_totalCalculations = 0;  ///< update()中的速率计算总次数
};

#endif // DATASTATISTICS_H
