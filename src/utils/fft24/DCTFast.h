/**
 * @file DCTFast.h
 * @brief 快速DCT实现 — Type II/III(AAN算法)/缩放/2D DCT
 *
 * 功能: 基于FFT的快速离散余弦变换，支持DCT-II/DCT-III，
 *       AAN快速算法，缩放/非缩放变换，以及2D DCT图像处理。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DataTransformer(数据变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 快速离散余弦变换引擎
 */
class DCTFast : public QObject {
    Q_OBJECT

public:
    /** @brief DCT类型 */
    enum class DCTType {
        TypeII,      ///< DCT-II(标准正向)
        TypeIII,     ///< DCT-III(标准反向/IDCT)
        TypeIIScaled ///< DCT-II缩放(正交)
    };
    Q_ENUM(DCTType)

    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计变换次数
        quint64 totalElementsProcessed = 0; ///< 累计处理元素数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     maxTransformSize = 0;       ///< 历史最大变换尺寸
    };

    explicit DCTFast(QObject* parent = nullptr);

    /** @brief 一维DCT变换 @param input 输入数据 @param type DCT类型 @return 变换结果 */
    QVector<double> transform(const QVector<double>& input,
                              DCTType type = DCTType::TypeII);

    /** @brief 一维逆DCT @param input DCT系数 @param type DCT类型 @return 逆变换结果 */
    QVector<double> inverseTransform(const QVector<double>& input,
                                     DCTType type = DCTType::TypeII);

    /** @brief 2D DCT变换 @param input 2D输入(行优先) @param rows 行数 @param cols 列数 @return 2D DCT系数 */
    QVector<double> transform2D(const QVector<double>& input,
                                int rows, int cols);

    /** @brief 2D逆DCT @param input 2D DCT系数 @param rows 行数 @param cols 列数 @return 2D逆变换 */
    QVector<double> inverseTransform2D(const QVector<double>& input,
                                       int rows, int cols);

    /** @brief 量化DCT系数(类似JPEG) @param coeffs DCT系数 @param quantTable 量化表 @return 量化后系数 */
    QVector<double> quantize(const QVector<double>& coeffs,
                             const QVector<double>& quantTable);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 变换尺寸 @param type DCT类型 */
    void transformComplete(int size, int type);

private:
    void fftBasedDCTII(QVector<double>& data);
    void fftBasedDCTIII(QVector<double>& data);
    void aanDCTII(QVector<double>& data);
    void baseFFT(QVector<double>& real, QVector<double>& imag);
    int nextPowerOf2(int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
