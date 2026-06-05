/**
 * @file DaubechiesWavelet.h
 * @brief Daubechies小波族引擎 — DB2~DB20正/逆变换
 *
 * 提供Daubechies小波族(DB2~DB20)的正变换和逆变换实现,
 * 支持任意长度的信号处理、多级分解、系数阈值去噪。
 * 适用于嵌入式调试中的信号分析和特征提取。
 */
#ifndef DAUBECHIES_WAVELET_H
#define DAUBECHIES_WAVELET_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QMap>

/**
 * @class DaubechiesWavelet
 * @brief Daubechies小波族引擎(DB2~DB20)
 *
 * 标准Daubechies小波变换, 使用对称延拓处理边界。
 */
class DaubechiesWavelet : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalForward = 0;       ///< 正变换次数
        quint64 totalInverse = 0;       ///< 逆变换次数
        quint64 totalCoeffs = 0;        ///< 处理的系数总数
        double  avgTimeMs = 0.0;        ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DaubechiesWavelet(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DaubechiesWavelet() override;

    // ── 配置 ──

    /**
     * @brief 设置小波阶数(DB2~DB20)
     * @param order 阶数(2的倍数, 2~20)
     */
    void setOrder(int order);

    /** @brief 获取当前小波阶数 */
    int order() const;

    // ── 变换 ──

    /**
     * @brief 多级正变换(分解)
     * @param signal 输入信号
     * @param levels 分解级数(0=自动计算最大级数)
     * @return 每级分解的近似+细节系数 [level][coeffs]
     */
    QVector<QVector<double>> forward(const QVector<double>& signal,
                                     int levels = 0);

    /**
     * @brief 多级逆变换(重构)
     * @param coefficients 多级分解系数
     * @param originalLength 原始信号长度
     * @return 重构后的信号
     */
    QVector<double> inverse(const QVector<QVector<double>>& coefficients,
                            int originalLength);

    /**
     * @brief 单级正变换
     * @param signal 输入信号
     * @return [近似系数, 细节系数]
     */
    QVector<QVector<double>> forwardOneLevel(const QVector<double>& signal);

    /**
     * @brief 单级逆变换
     * @param approx 近似系数
     * @param detail 细节系数
     * @param outputLength 输出长度
     * @return 重构信号
     */
    QVector<double> inverseOneLevel(const QVector<double>& approx,
                                    const QVector<double>& detail,
                                    int outputLength);

    // ── 去噪 ──

    /**
     * @brief 软阈值去噪
     * @param signal 输入信号
     * @param threshold 阈值
     * @param levels 分解级数
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal,
                            double threshold, int levels = 0);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 正变换完成信号 @param levels 级数 @param count 系数数量 */
    void forwardCompleted(int levels, int count);
    /** @brief 逆变换完成信号 @param length 重构信号长度 */
    void inverseCompleted(int length);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 加载Daubechies滤波器系数 */
    void loadFilters(int order);

    /** @brief 周期延拓卷积(正变换一步) */
    void convolveForward(const QVector<double>& signal,
                         QVector<double>& approx,
                         QVector<double>& detail) const;

    /** @brief 周期延拓卷积(逆变换一步) */
    void convolveInverse(const QVector<double>& approx,
                         const QVector<double>& detail,
                         QVector<double>& output) const;

    /** @brief 软阈值函数 */
    static double softThreshold(double value, double threshold);

    int m_order = 4;                        ///< 当前小波阶数
    QVector<double> m_decompLow;            ///< 分解低通滤波器
    QVector<double> m_decompHigh;           ///< 分解高通滤波器
    QVector<double> m_reconLow;             ///< 重构低通滤波器
    QVector<double> m_reconHigh;            ///< 重构高通滤波器

    mutable Stats m_stats;                  ///< 操作统计
};

#endif // DAUBECHIES_WAVELET_H
