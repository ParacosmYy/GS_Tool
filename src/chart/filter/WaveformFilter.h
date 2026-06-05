/**
 * @file WaveformFilter.h
 * @brief 波形数字滤波器 -- 8种DSP滤波算法,支持级联处理与频率响应估计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持算法: MovingAverage / Median / Exponential / LowPass / HighPass /
 *           BandPass / BandStop / SavitzkyGolay
 * 线程安全: 可重入,非线程安全;多线程环境需外部同步。
 */

#ifndef WAVEFORMFILTER_H
#define WAVEFORMFILTER_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVector>

/**
 * @class WaveformFilter
 * @brief 波形数字滤波器,8种DSP算法,级联处理,频率响应估计
 *
 * 用法: setFilterParams() → apply() / applyCascade()
 * 参数键: LowPass/HighPass("cutoffFreq","sampleRate") BandPass/BandStop("lowCutoff","highCutoff","sampleRate")
 *         MovingAverage/Median("windowSize") Exponential("alpha") SavitzkyGolay("windowSize","polyOrder")
 */
class WaveformFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型枚举 */
    enum class FilterType {
        LowPass        = 0,  ///< 低通(RC单极IIR)
        HighPass       = 1,  ///< 高通(RC单极IIR)
        BandPass       = 2,  ///< 带通(低通+高通级联)
        BandStop       = 3,  ///< 带阻(低通与高通差分)
        MovingAverage  = 4,  ///< 滑动平均
        Median         = 5,  ///< 中值滤波
        SavitzkyGolay  = 6,  ///< Savitzky-Golay多项式拟合
        Exponential    = 7   ///< 指数移动平均(EMA)
    };
    Q_ENUM(FilterType)

    /** @brief 级联步骤: 类型+参数 */
    struct CascadeStep {
        FilterType type;        ///< 滤波器类型
        QVariantMap params;     ///< 滤波器参数
    };

    /** @brief 运行统计快照 */
    struct Stats {
        quint64 totalFiltersApplied    = 0;   ///< 累计滤波次数
        quint64 totalSamplesProcessed  = 0;   ///< 累计采样点数
        double  avgProcessingTimeMs    = 0.0; ///< 平均耗时(ms)
        quint64 totalCascadeOperations = 0;   ///< 累计级联次数
        QMap<int, quint64> filtersByType;     ///< 各类型执行计数[枚举值→次数]
    };

    explicit WaveformFilter(QObject *parent = nullptr);
    ~WaveformFilter() override;
    WaveformFilter(const WaveformFilter &) = delete;
    WaveformFilter &operator=(const WaveformFilter &) = delete;

    // ── 参数配置 ──

    /** @brief 设置主滤波器参数 @sa 参数键名见类注释 */
    void setFilterParams(FilterType type, const QVariantMap &params);

    /** @brief 当前主滤波器类型 */
    FilterType filterType() const;

    /** @brief 当前主滤波器参数 */
    QVariantMap filterParams() const;

    // ── 单次滤波 ──

    /** @brief 应用主滤波器,返回滤波后数据(不修改原数据) */
    QVector<double> apply(const QVector<double> &data);

    /** @brief 就地应用主滤波器 @return true=成功 */
    bool applyInPlace(QVector<double> &data);

    // ── 级联滤波 ──

    /** @brief 追加级联步骤 */
    void cascadeAppend(FilterType type, const QVariantMap &params);

    /** @brief 清空级联链 */
    void cascadeClear();

    /** @brief 获取级联链 */
    QVector<CascadeStep> cascadeChain() const;

    /** @brief 按级联链依次滤波 */
    QVector<double> applyCascade(const QVector<double> &data);

    // ── 系数与频率响应 ──

    /** @brief 获取主滤波器系数(LowPass/HighPass→[b0,a1], Exponential→[alpha,1-alpha], MovingAverage→均值向量) */
    QVector<double> coefficients() const;

    /** @brief 估计主滤波器幅频响应 @param freqPoints 频率点(Hz) @return 归一化幅度(0~1) */
    QVector<double> frequencyResponse(const QVector<double> &freqPoints) const;

    // ── 统计 ──

    /** @brief 统计快照 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波完成信号 @param samplesProcessed 采样点数 @param processingTimeMs 耗时(ms) */
    void filterApplied(int samplesProcessed, double processingTimeMs);

    /** @brief 错误信号 @param message 错误描述 */
    void error(const QString &message);

private:
    QVector<double> filterMovingAverage(const QVector<double> &data, int windowSize);
    QVector<double> filterMedian(const QVector<double> &data, int windowSize);
    QVector<double> filterExponential(const QVector<double> &data, double alpha);
    QVector<double> filterLowPass(const QVector<double> &data, double cutoffFreq, double sampleRate);
    QVector<double> filterHighPass(const QVector<double> &data, double cutoffFreq, double sampleRate);
    QVector<double> filterBandPass(const QVector<double> &data, double lowCut, double highCut, double sr);
    QVector<double> filterBandStop(const QVector<double> &data, double lowCut, double highCut, double sr);
    QVector<double> filterSavitzkyGolay(const QVector<double> &data, int windowSize, int polyOrder);
    QVector<double> dispatchFilter(FilterType type, const QVariantMap &params, const QVector<double> &data);

    FilterType m_filterType = FilterType::MovingAverage;  ///< 主滤波器类型
    QVariantMap m_filterParams;                            ///< 主滤波器参数
    QVector<CascadeStep> m_cascadeChain;                   ///< 级联链

    quint64 m_totalFiltersApplied   = 0;   ///< 滤波次数
    quint64 m_totalSamplesProcessed = 0;   ///< 采样点数
    quint64 m_totalCascadeOps       = 0;   ///< 级联次数
    double  m_totalProcessingTimeMs = 0.0; ///< 累计耗时(ms)
    quint64 m_timeCount             = 0;   ///< 耗时采样次数
    QMap<int, quint64> m_filtersByType;    ///< 各类型计数
};

#endif // WAVEFORMFILTER_H
