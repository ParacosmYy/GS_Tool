/**
 * @file DataRateCalculator.h
 * @brief 数据速率计算器 -- 实时追踪串口通信吞吐率与统计信息
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供可配置的滑动时间窗口(1/5/10/30/60秒)内的实时速率计算，
 * 支持 TX/RX 双向独立统计、峰值追踪、指数移动平均平滑、
 * 环形缓冲区采样和 CSV 导出。
 */

#ifndef DATARATECALCULATOR_H
#define DATARATECALCULATOR_H

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>

/**
 * @class DataRateCalculator
 * @brief 数据速率计算器，实时追踪通信双向吞吐率
 *
 * 核心设计：
 * - TX(发送) 和 RX(接收) 双向独立统计
 * - 环形缓冲区保存最近 N 秒的采样点，用于滑动窗口速率计算
 * - 指数移动平均(EMA)平滑速率，降低抖动
 * - 峰值速率自动追踪，带时间戳
 * - 支持导出完整历史到 CSV
 */
class DataRateCalculator : public QObject {
    Q_OBJECT

public:
    /** @brief 滑动窗口大小(秒) */
    enum class WindowSize {
        Sec1  = 1,   ///< 1 秒窗口
        Sec5  = 5,   ///< 5 秒窗口
        Sec10 = 10,  ///< 10 秒窗口
        Sec30 = 30,  ///< 30 秒窗口
        Sec60 = 60   ///< 60 秒窗口
    };
    Q_ENUM(WindowSize)

    /** @brief 单次速率采样点 */
    struct RateSample {
        qint64  timestamp = 0;  ///< 采样时间戳(ms since epoch)
        quint64 bytes     = 0;  ///< 累计字节数
        double  rateBps   = 0.0; ///< 此时刻瞬时速率(Bytes/s)
        double  rateKbps  = 0.0; ///< 此时刻瞬时速率(KBits/s)
    };

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalSamples    = 0;  ///< 总采样数
        quint64 totalBytesTx    = 0;  ///< 累计发送字节数
        quint64 totalBytesRx    = 0;  ///< 累计接收字节数
        double  peakRateTx      = 0.0; ///< 发送峰值速率(Bytes/s)
        double  peakRateRx      = 0.0; ///< 接收峰值速率(Bytes/s)
        double  currentRateTx   = 0.0; ///< 当前发送速率(Bytes/s)
        double  currentRateRx   = 0.0; ///< 当前接收速率(Bytes/s)
        quint64 windowSizeMs    = 0;   ///< 当前滑动窗口大小(ms)
        quint64 totalOverflows  = 0;   ///< 环形缓冲区溢出次数
        quint64 totalUnderruns  = 0;   ///< 窗口内无数据次数
    };

    /**
     * @brief 构造数据速率计算器
     * @param parent 父对象
     */
    explicit DataRateCalculator(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DataRateCalculator() override;

    // ── 数据记录接口 ──

    /** @brief 记录发送字节数 @param bytes 本次发送的字节数 */
    void recordTx(quint64 bytes);

    /** @brief 记录接收字节数 @param bytes 本次接收的字节数 */
    void recordRx(quint64 bytes);

    // ── 速率查询接口 ──

    /** @brief 获取当前发送速率(Bytes/s) @return EMA 平滑后的速率 */
    double currentRateTx() const { return m_emaRateTx; }

    /** @brief 获取当前接收速率(Bytes/s) @return EMA 平滑后的速率 */
    double currentRateRx() const { return m_emaRateRx; }

    /** @brief 获取发送方向的历史采样列表 @return 采样列表(时间升序) */
    QList<RateSample> samplesTx() const;

    /** @brief 获取接收方向的历史采样列表 @return 采样列表(时间升序) */
    QList<RateSample> samplesRx() const;

    // ── 配置接口 ──

    /** @brief 设置滑动窗口大小 @param size 窗口大小枚举 */
    void setWindowSize(WindowSize size);

    /** @brief 获取当前窗口大小 @return 窗口大小枚举 */
    WindowSize windowSize() const { return m_windowSize; }

    /** @brief 设置 EMA 平滑系数(0.0~1.0) @param alpha 平滑系数，默认 0.3 */
    void setSmoothingAlpha(double alpha);

    // ── 导出与重置 ──

    /** @brief 重置当前窗口内的采样缓冲区(不影响累计统计) */
    void resetWindow();

    /**
     * @brief 导出历史采样到 CSV 文件
     * @param filePath 目标文件路径
     * @return true 导出成功
     */
    bool exportHistory(const QString &filePath) const;

    /** @brief 获取全局统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有计数器、历史和统计信息 */
    void resetStatistics();

signals:
    /**
     * @brief 速率更新信号，每秒发射一次
     * @param rateTx 当前发送速率(Bytes/s)
     * @param rateRx 当前接收速率(Bytes/s)
     */
    void rateUpdated(double rateTx, double rateRx);

    /**
     * @brief 峰值速率变更信号
     * @param direction 方向("TX" 或 "RX")
     * @param newPeakRate 新的峰值速率(Bytes/s)
     */
    void peakRateChanged(const QString &direction, double newPeakRate);

private slots:
    /** @brief 每秒定时器回调：计算速率、更新统计 */
    void onTick();

private:
    /**
     * @brief 向环形缓冲区推入采样点
     * @param ring 目标环形缓冲区
     * @param sample 采样数据
     */
    void pushSample(QList<RateSample> &ring, const RateSample &sample);

    /**
     * @brief 从环形缓冲区计算滑动窗口速率
     * @param ring 采样环形缓冲区
     * @param nowMs 当前时间戳(ms)
     * @return 窗口内平均速率(Bytes/s)
     */
    double calcWindowRate(const QList<RateSample> &ring, qint64 nowMs) const;

    /** @brief 移除环形缓冲区中过期的采样点 @param ring 目标缓冲区 @param cutoffMs 截止时间戳 */
    void pruneExpired(QList<RateSample> &ring, qint64 cutoffMs);

    // ── 定时器 ──
    QTimer       m_tickTimer;          ///< 每秒采样定时器
    QElapsedTimer m_elapsed;           ///< 运行计时器

    // ── 窗口配置 ──
    WindowSize   m_windowSize = WindowSize::Sec5; ///< 当前窗口大小
    double       m_alpha      = 0.3;    ///< EMA 平滑系数

    // ── TX 环形缓冲区 ──
    QList<RateSample> m_ringTx;         ///< TX 采样环形缓冲区
    quint64      m_accumTx   = 0;       ///< 当前窗口累计 TX 字节
    double       m_emaRateTx = 0.0;     ///< TX EMA 平滑速率
    double       m_peakTx    = 0.0;     ///< TX 峰值速率

    // ── RX 环形缓冲区 ──
    QList<RateSample> m_ringRx;         ///< RX 采样环形缓冲区
    quint64      m_accumRx   = 0;       ///< 当前窗口累计 RX 字节
    double       m_emaRateRx = 0.0;     ///< RX EMA 平滑速率
    double       m_peakRx    = 0.0;     ///< RX 峰值速率

    // ── 全局统计计数器 ──
    quint64 m_totalSamples    = 0;      ///< 累计采样次数
    quint64 m_totalBytesTx    = 0;      ///< 累计发送字节总数
    quint64 m_totalBytesRx    = 0;      ///< 累计接收字节总数
    quint64 m_totalOverflows  = 0;      ///< 环形缓冲区溢出次数
    quint64 m_totalUnderruns  = 0;      ///< 窗口内无数据次数

    static constexpr int kMaxRingSize = 3600; ///< 环形缓冲区最大容量(支持 60s * 60min)
};

#endif // DATARATECALCULATOR_H
