/**
 * @file HaarWavelet2D.h
 * @brief 二维Haar小波变换 — 图像/矩阵处理
 *
 * 功能: 2D Haar正变换/逆变换，软阈值去噪，
 *       统计变换次数/耗时，变换完成信号。
 */
#ifndef HAARWAVELET2D_H
#define HAARWAVELET2D_H

#include <QObject>
#include <QVector>

class HaarWavelet2D : public QObject {
    Q_OBJECT
public:
    /** 二维矩阵(行优先存储) */
    using Matrix = QVector<QVector<double>>;

    /** 操作统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit HaarWavelet2D(QObject* parent = nullptr);

    /** @brief 2D Haar正变换 @param matrix 输入矩阵(行列须为2的幂) @return 变换系数 */
    Matrix forward(const Matrix& matrix);

    /** @brief 2D Haar逆变换 @param coeffs 变换系数 @return 重构矩阵 */
    Matrix inverse(const Matrix& coeffs);

    /** @brief 软阈值去噪 @param matrix 输入矩阵 @param threshold 阈值 @return 去噪后矩阵 */
    Matrix denoise(const Matrix& matrix, double threshold);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param rows 行数 @param cols 列数 */
    void transformCompleted(int rows, int cols);

private:
    static Matrix forwardRows(const Matrix& m);
    static Matrix forwardCols(const Matrix& m);
    static Matrix inverseRows(const Matrix& m);
    static Matrix inverseCols(const Matrix& m);
    static Matrix transpose(const Matrix& m);

    Stats m_stats;
    double m_timeSum;
};

#endif // HAARWAVELET2D_H
