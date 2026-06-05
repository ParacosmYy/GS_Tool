/**
 * @file DaubechiesD4.h
 * @brief Daubechies-4小波变换引擎 — 正/逆变换与阈值去噪
 *
 * 提供Daubechies-4阶小波的正向变换、逆向重建和软/硬阈值去噪,
 * 适用于信号分析、噪声抑制和特征提取等嵌入式调试场景。
 */
#ifndef DAUBECHIESD4_H
#define DAUBECHIESD4_H

#include <QObject>
#include <QVector>

/**
 * @class DaubechiesD4
 * @brief Daubechies-4小波变换 — 正/逆变换+阈值去噪
 *
 * 典型用法:
 * @code
 *   DaubechiesD4 dwt;
 *   auto coeffs = dwt.forward(signal);
 *   auto denoised = dwt.denoise(signal, 0.1);
 *   auto recovered = dwt.inverse(coeffs);
 * @endcode
 */
class DaubechiesD4 : public QObject {
    Q_OBJECT

public:
    /** @brief 去噪阈值类型 */
    enum class ThresholdType {
        Soft = 0,  ///< 软阈值: 系数向零收缩
        Hard       ///< 硬阈值: 小于阈值直接置零
    };
    Q_ENUM(ThresholdType)

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalTransforms = 0;     ///< 总变换次数(正+逆+去噪)
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DaubechiesD4(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DaubechiesD4() override;

    // ── 核心接口 ──

    /**
     * @brief 正向Daubechies-4小波变换
     * @param signal 输入信号(长度自动补齐2的幂)
     * @return 小波系数序列[近似+细节]
     */
    QVector<double> forward(const QVector<double>& signal);

    /**
     * @brief 逆向Daubechies-4小波变换
     * @param coeffs 小波系数序列(由forward产生)
     * @return 重建信号
     */
    QVector<double> inverse(const QVector<double>& coeffs);

    /**
     * @brief 阈值去噪
     * @param signal 输入信号
     * @param threshold 阈值大小
     * @param type 阈值类型(默认软阈值)
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal, double threshold,
                            ThresholdType type = ThresholdType::Soft);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 正向变换完成信号 @param size 输出系数数量 */
    void forwardCompleted(int size);
    /** @brief 逆向变换完成信号 @param size 重建信号长度 */
    void inverseCompleted(int size);
    /** @brief 去噪完成信号 @param size 输出信号长度 */
    void denoiseCompleted(int size);

private:
    /** @brief 将长度补齐到最近的2的幂 */
    int padToPowerOf2(QVector<double>& data) const;

    /** @brief 单层正向D4变换 */
    void forwardPass(QVector<double>& data, int len);

    /** @brief 单层逆向D4变换 */
    void inversePass(QVector<double>& data, int len);

    /** @brief D4分解低通滤波器系数(4个) */
    static constexpr double S_LP[4] = {
        0.6830127,  1.1830127, 0.3169873, -0.1830127
    };

    /** @brief D4分解高通滤波器系数(4个) */
    static constexpr double S_HP[4] = {
        -0.1830127, -0.3169873, 1.1830127, -0.6830127
    };

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // DAUBECHIESD4_H
