/**
 * @file RealFFT.h
 * @brief 实值快速傅里叶变换(RFFT) — 针对实数输入优化的FFT
 *
 * 提供针对实数输入优化的FFT/IFFT实现, 利用实数信号的
 * 共轭对称性将计算量减半, 适用于嵌入式调试场景中的
 * 信号频谱分析、滤波和特征提取。
 */
#ifndef REAL_FFT_H
#define REAL_FFT_H

#include <QObject>
#include <QVector>
#include <utility>

/**
 * @class RealFFT
 * @brief 实值快速傅里叶变换(RFFT)
 *
 * 典型用法:
 * @code
 *   RealFFT fft;
 *   auto spectrum = fft.forward(timeDomain);
 *   auto recovered = fft.inverse(spectrum);
 * @endcode
 */
class RealFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 复数对(实部, 虚部) */
    using Complex = std::pair<double, double>;

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalFFTs = 0;              ///< 正变换操作总次数
        quint64 totalIFFTs = 0;             ///< 逆变换操作总次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit RealFFT(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~RealFFT() override;

    // ── 变换 ──

    /**
     * @brief 实值FFT正变换
     * @param input 实数输入序列(长度自动补零到2的幂)
     * @return 频谱复数序列(共轭对称, 长度N/2+1)
     */
    QVector<Complex> forward(const QVector<double>& input);

    /**
     * @brief 实值FFT逆变换
     * @param spectrum 频谱复数序列
     * @return 重建的实数序列
     */
    QVector<double> inverse(const QVector<Complex>& spectrum);

    // ── 辅助 ──

    /**
     * @brief 计算幅度谱
     * @param spectrum 频谱复数序列
     * @return 幅度值序列
     */
    QVector<double> magnitude(const QVector<Complex>& spectrum) const;

    /**
     * @brief 计算功率谱密度
     * @param spectrum 频谱复数序列
     * @return 功率值序列
     */
    QVector<double> powerSpectrum(const QVector<Complex>& spectrum) const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief FFT完成信号 @param binCount 频率bin数 */
    void forwardCompleted(int binCount);
    /** @brief IFFT完成信号 @param sampleCount 重建采样数 */
    void inverseCompleted(int sampleCount);

private:
    /**
     * @brief Cooley-Tukey基2蝶形运算
     * @param re 实部数组
     * @param im 虚部数组
     * @param inverse 是否逆变换
     */
    void butterfly(QVector<double>& re, QVector<double>& im,
                   bool inverse) const;

    /**
     * @brief 计算下一个>=n的2的幂
     * @param n 输入值
     * @return 2的幂
     */
    int nextPowerOf2(int n) const;

    /**
     * @brief 位反转重排
     * @param data 数据数组
     * @param n 数据长度(2的幂)
     */
    void bitReverse(QVector<double>& data, int n) const;

    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

#endif // REAL_FFT_H
