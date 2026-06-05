/**
 * @file DiscreteCosine3.h
 * @brief 离散余弦变换增强 — DCT-I/II/III/IV/快速算法/2D变换
 *
 * 功能: 支持DCT-I/II/III/IV四种类型，含快速算法实现，
 *       支持正向/逆向变换、2D-DCT变换、频域能量集中分析。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / WaveformGenerator(信号生成)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QVector2D>

/**
 * @brief 离散余弦变换增强 — DCT-I/II/III/IV + 2D
 */
class DiscreteCosine3 : public QObject {
    Q_OBJECT

public:
    /** @brief DCT类型 */
    enum class DctType {
        TypeI,   ///< DCT-I (边界条件: 对称)
        TypeII,  ///< DCT-II (标准DCT, JPEG使用)
        TypeIII, ///< DCT-III (DCT-II的逆)
        TypeIV   ///< DCT-IV (MDCT基础)
    };
    Q_ENUM(DctType)

    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;          ///< 累计变换次数
        quint64 totalSamplesProcessed = 0;    ///< 累计处理采样数
        double  avgProcessingTimeMs = 0.0;    ///< 平均处理时间(ms)
        double  totalEnergyRatio = 0.0;       ///< 平均能量集中比
    };

    explicit DiscreteCosine3(QObject* parent = nullptr);

    /** @brief 正向DCT @param input 输入信号 @param type DCT类型 @return 变换结果 */
    QVector<double> forward(const QVector<double>& input,
                            DctType type = DctType::TypeII);

    /** @brief 逆向DCT @param input 频域系数 @param type DCT类型 @return 时域信号 */
    QVector<double> inverse(const QVector<double>& input,
                            DctType type = DctType::TypeII);

    /** @brief 2D-DCT @param matrix 输入矩阵(行优先) @param rows 行数 @param cols 列数 @return 变换矩阵 */
    QVector<double> forward2D(const QVector<double>& matrix,
                              int rows, int cols);

    /** @brief 2D-IDCT @param matrix 频域矩阵 @param rows 行数 @param cols 列数 @return 时域矩阵 */
    QVector<double> inverse2D(const QVector<double>& matrix,
                              int rows, int cols);

    /** @brief 能量集中比 @param coeffs DCT系数 @param keepRatio 保留系数比例(0~1) @return 保留能量占比 */
    double energyConcentration(const QVector<double>& coeffs,
                               double keepRatio = 0.25) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param sampleCount 采样数 @param type DCT类型 */
    void transformComplete(int sampleCount, DctType type);

private:
    QVector<double> dctTypeI(const QVector<double>& input, bool inverse) const;
    QVector<double> dctTypeII(const QVector<double>& input, bool inverse) const;
    QVector<double> dctTypeIII(const QVector<double>& input, bool inverse) const;
    QVector<double> dctTypeIV(const QVector<double>& input, bool inverse) const;
    QVector<double> fastDctII(const QVector<double>& input) const;
    QVector<double> fastDctIII(const QVector<double>& input) const;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_energyRatioSum = 0.0;
};
