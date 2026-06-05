/**
 * @file DctTransform.h
 * @brief 离散余弦变换 (DCT) — DCT-I/II/III/IV 四种类型
 *
 * 功能: 基于基 2 FFT 实现四种离散余弦变换，支持信号压缩、
 *       频域特征提取和 JPEG/MPEG 编码预处理。
 *       DCT-II 为标准形式，DCT-III 为其逆变换。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / SignalDecomposer(信号分解)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换引擎
 *
 * 支持四种 DCT 类型:
 *   DCT-I   — 对称边界条件，用于数值求解
 *   DCT-II  — 标准压缩DCT，JPEG/MPEG 核心
 *   DCT-III — DCT-II 的逆变换 (IDCT)
 *   DCT-IV  — 正交变换，用于 MDCT 的基础
 */
class DctTransform : public QObject {
    Q_OBJECT

public:
    /** @brief DCT 类型 */
    enum class DctType {
        TypeI = 1,      ///< DCT-I: 对称边界 (y(-1)=y(N-2), y(N)=y(1))
        TypeII = 2,     ///< DCT-II: 标准压缩DCT (最常用)
        TypeIII = 3,    ///< DCT-III: DCT-II 的逆变换
        TypeIV = 4      ///< DCT-IV: 正交DCT，MDCT 基础
    };
    Q_ENUM(DctType)

    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;            ///< 累计变换次数
        int totalPointsProcessed = 0;       ///< 累计处理数据点数
        int totalFFTCalls = 0;              ///< 累计 FFT 调用次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit DctTransform(QObject* parent = nullptr);

    /**
     * @brief 执行正向 DCT 变换
     * @param input 输入时域数据 (长度建议为 2 的幂)
     * @param type DCT 类型
     * @return 变换结果 (频域系数)
     */
    QVector<double> forward(const QVector<double>& input, DctType type = DctType::TypeII);

    /**
     * @brief 执行逆向 DCT 变换 (IDCT)
     * @param coefficients DCT 系数
     * @param type DCT 类型 (应与正向变换一致)
     * @return 逆变换结果 (时域数据)
     */
    QVector<double> inverse(const QVector<double>& coefficients,
                            DctType type = DctType::TypeII);

    /**
     * @brief 提取低频系数 (截断高频)
     * @param coefficients DCT 系数
     * @param keepCount 保留的系数个数
     * @return 截断后的系数
     */
    QVector<double> truncateCoefficients(const QVector<double>& coefficients,
                                         int keepCount) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 数据长度 @param type DCT类型 */
    void transformCompleted(int size, DctType type);

private:
    void fft(QVector<double>& real, QVector<double>& imag);
    QVector<double> dctTypeI(const QVector<double>& input);
    QVector<double> dctTypeII(const QVector<double>& input);
    QVector<double> dctTypeIII(const QVector<double>& input);
    QVector<double> dctTypeIV(const QVector<double>& input);

    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
