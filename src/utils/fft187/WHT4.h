/**
 * @file WHT4.h
 * @brief 沃尔什-哈达玛变换(序列序蝶形+原位O(N log N)) — Walsh-Hadamard Transform via Sequency-Ordered Butterfly with Fast In-Place O(N log N)
 *
 * 功能: 实现沃尔什-哈达玛变换，支持自然序/序列序蝶形运算、
 *       原位快速计算、2D变换和功率谱估计。
 *
 * 协作: DCT4(DCT-IV) / DST4(DST-IV) / FFT2(快速傅里叶)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换器(序列序蝶形)
 */
class WHT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        bool sequencyOrder = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT4(QObject *parent = nullptr);
    ~WHT4() override;

    void setSequencyOrder(bool enabled);

    /** @brief 正向WHT变换 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief 逆向WHT变换(WHT是自逆的, 需缩放) */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief 自然序快速蝶形WHT */
    QVector<double> naturalOrder(const QVector<double>& input) const;

    /** @brief 序列序快速蝶形WHT */
    QVector<double> sequencyOrderWHT(const QVector<double>& input) const;

    /** @brief 2D WHT变换 */
    QVector<QVector<double>> transform2D(const QVector<QVector<double>>& input);

    /** @brief 计算Walsh功率谱 */
    QVector<double> powerSpectrum(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    bool m_sequencyOrder = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bit-reversal permutation for sequency ordering */
    void bitReversePermute(QVector<double>& data) const;

    /** @brief Compute Gray code rank for sequency ordering */
    int grayCodeRank(int index) const;

    /** @brief In-place Hadamard butterfly */
    void hadamardButterfly(QVector<double>& data) const;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;
};
