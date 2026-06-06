/**
 * @file MixedRadixFFT4.h
 * @brief 混合基FFT(基2/3/4/5多级组合复合长度变换) — Mixed-radix FFT Combining Radix-2/3/4/5 Stages for Composite-length Transforms
 *
 * 功能: 实现混合基FFT算法，支持基2/3/4/5组合分解，
 *       适用于复合长度(N=2^a*3^b*4^c*5^d)的快速傅里叶变换。
 *
 * 协作: BruunFFT4(Bruun FFT) / SplitRadixFFT4(分裂基) / WinogradFFT4(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 混合基FFT处理器(基2/3/4/5组合)
 */
class MixedRadixFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT4(QObject *parent = nullptr);
    ~MixedRadixFFT4() override;

    /** @brief Forward transform (complex I/O) */
    void transform(const QVector<double>& inRe, const QVector<double>& inIm,
                   QVector<double>& outRe, QVector<double>& outIm);

    /** @brief Inverse transform */
    void inverseTransform(const QVector<double>& inRe, const QVector<double>& inIm,
                          QVector<double>& outRe, QVector<double>& outIm);

    /** @brief 检查N是否可分解为2^a*3^b*5^c */
    bool isCompositeLength(int n) const;

    /** @brief 分解N为质因数(仅2,3,5) */
    QVector<int> factorize(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int stages);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive mixed-radix Cooley-Tukey */
    void mixedRadixCT(const QVector<double>& inRe, const QVector<double>& inIm,
                      QVector<double>& outRe, QVector<double>& outIm,
                      int N, int stride, int offset);

    /** @brief Radix-2 butterfly */
    void radix2(const QVector<double>& inRe, const QVector<double>& inIm,
                QVector<double>& outRe, QVector<double>& outIm,
                int N, int stride, int offset);

    /** @brief Radix-3 butterfly */
    void radix3(const QVector<double>& inRe, const QVector<double>& inIm,
                QVector<double>& outRe, QVector<double>& outIm,
                int N, int stride, int offset);

    /** @brief Radix-4 butterfly */
    void radix4(const QVector<double>& inRe, const QVector<double>& inIm,
                QVector<double>& outRe, QVector<double>& outIm,
                int N, int stride, int offset);

    /** @brief Radix-5 butterfly */
    void radix5(const QVector<double>& inRe, const QVector<double>& inIm,
                QVector<double>& outRe, QVector<double>& outIm,
                int N, int stride, int offset);
};
