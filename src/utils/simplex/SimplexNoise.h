/**
 * @file SimplexNoise.h
 * @brief Simplex噪声生成器 — 2D/3D梯度噪声
 *
 * 功能: 实现Simplex噪声(改进版Perlin噪声)，支持2D/3D噪声、
 *       分形叠加(octaves)、频率/振幅控制，统计生成次数/耗时。
 */
#ifndef SIMPLEXNOISE_H
#define SIMPLEXNOISE_H

#include <QObject>
#include <QVector>

class SimplexNoise : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalSamples = 0;
        quint64 totalOctaveSamples = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SimplexNoise(int seed = 0, QObject* parent = nullptr);

    /** @brief 2D噪声 @param x @param y @return 噪声值[-1,1] */
    double noise2D(double x, double y) const;

    /** @brief 3D噪声 @param x @param y @param z @return 噪声值[-1,1] */
    double noise3D(double x, double y, double z) const;

    /** @brief 分形2D噪声 @param x @param y @param octaves 叠加层数 @param persistence 持续度 @return 噪声值 */
    double fractal2D(double x, double y, int octaves = 4,
                     double persistence = 0.5) const;

    /** @brief 生成2D噪声图 @param width 宽 @param height 高 @param scale 缩放 @return 噪声矩阵(行优先) */
    QVector<double> generate2DMap(int width, int height,
                                   double scale = 0.01);

    void setSeed(int seed);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mapGenerated(int width, int height);

private:
    double grad2(int hash, double x, double y) const;
    double grad3(int hash, double x, double y, double z) const;

    int m_perm[512];
    mutable Stats m_stats;
};

#endif // SIMPLEXNOISE_H
