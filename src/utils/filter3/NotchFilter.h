/**
 * @file NotchFilter.h
 * @brief IIR陷波(带阻)滤波器 — 2阶双二阶节实现
 *
 * 设计并应用IIR陷波滤波器, 可指定中心频率、Q值和采样率,
 * 适用于工频干扰消除、窄带噪声抑制等嵌入式信号调理场景。
 */
#ifndef NOTCHFILTER_H
#define NOTCHFILTER_H

#include <QObject>
#include <QVector>

/**
 * @class NotchFilter
 * @brief IIR陷波滤波器 — 可调中心频率/Q值的带阻滤波
 *
 * 典型用法:
 * @code
 *   NotchFilter filter;
 *   filter.design(50.0, 30.0, 1000.0); // 50Hz陷波
 *   auto output = filter.apply(signal);
 * @endcode
 */
class NotchFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器参数结构 */
    struct Parameters {
        double centerFreqHz = 0.0;  ///< 中心频率(Hz)
        double qualityFactor = 1.0; ///< Q值(品质因数)
        double sampleRateHz = 0.0;  ///< 采样率(Hz)
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalDesigns = 0;        ///< 滤波器设计次数
        quint64 totalApplications = 0;   ///< 滤波器应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit NotchFilter(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~NotchFilter() override;

    // ── 核心接口 ──

    /**
     * @brief 设计陷波滤波器参数
     * @param freqHz 中心频率(Hz)
     * @param Q 品质因数(越大带宽越窄)
     * @param sampleRate 采样率(Hz)
     */
    void design(double freqHz, double Q, double sampleRate);

    /**
     * @brief 对信号应用陷波滤波
     * @param signal 输入信号
     * @return 滤波后信号
     */
    QVector<double> apply(const QVector<double>& signal);

    /**
     * @brief 对单个采样点进行滤波(Direct Form II Transposed)
     * @param sample 输入采样值
     * @return 滤波后采样值
     */
    double processSample(double sample);

    /** @brief 重置滤波器内部状态(清零延迟线) */
    void resetState();

    // ── 查询 ──

    /** @brief 获取当前滤波器参数 */
    Parameters parameters() const;

    /** @brief 滤波器是否已设计(可用) */
    bool isDesigned() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成信号 @param freqHz 中心频率 */
    void designCompleted(double freqHz);
    /** @brief 滤波应用完成信号 @param size 输出长度 */
    void applyCompleted(int size);

private:
    /** @brief 双二阶节系数 — 分子(b0, b1, b2) */
    double m_b0 = 1.0, m_b1 = 0.0, m_b2 = 1.0;

    /** @brief 双二阶节系数 — 分母(a1, a2, a0归一化为1) */
    double m_a1 = 0.0, m_a2 = 0.0;

    /** @brief 延迟线状态(Direct Form I) */
    double m_x1 = 0.0, m_x2 = 0.0;
    double m_y1 = 0.0, m_y2 = 0.0;

    /** @brief 是否已设计 */
    bool m_designed = false;

    /** @brief 当前参数 */
    Parameters m_params;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // NOTCHFILTER_H
