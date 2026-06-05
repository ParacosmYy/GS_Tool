/**
 * @file HaarWavelet3D.cpp
 * @brief 三维Haar小波变换实现 — 多尺度分解/重构/阈值去噪
 *
 * 逐层沿X/Y/Z三个轴方向分离低频近似和高频细节，
 * 支持多尺度分解与重构，提供硬阈值和软阈值去噪功能。
 */

#include "utils/haar3/HaarWavelet3D.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
HaarWavelet3D::HaarWavelet3D(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("HaarWavelet3D"));
}

// ── 数据设置 ──

/**
 * @brief 设置输入体数据
 * @param data 展平的3D数据(行优先, 大小须为nx*ny*nz)
 * @param nx X方向尺寸
 * @param ny Y方向尺寸
 * @param nz Z方向尺寸
 */
void HaarWavelet3D::setData(const QVector<double>& data,
                             int nx, int ny, int nz)
{
    m_nx = qMax(1, nx);
    m_ny = qMax(1, ny);
    m_nz = qMax(1, nz);
    m_data = data;
    int expected = m_nx * m_ny * m_nz;
    if (m_data.size() != expected) {
        m_data.resize(expected, 0.0);
    }
}

// ── 前向变换 ──

/**
 * @brief 执行多尺度前向Haar小波变换
 * @param levels 分解层数(0=自动计算)
 * @return 小波系数(含7个频带分量)
 *
 * 每层分解沿X/Y/Z三轴分离，产生7个子带:
 * 近似(LLL) + 6个细节(LLH,LHL,LHH,HLL,HLH,HHL)
 */
HaarWavelet3D::Coefficients HaarWavelet3D::forward(int levels)
{
    Coefficients result;
    int totalVoxels = m_nx * m_ny * m_nz;
    if (totalVoxels < 1) return result;

    QElapsedTimer timer;
    timer.start();

    /* 计算最大分解层数 */
    int minDim = qMin({m_nx, m_ny, m_nz});
    int maxLevels = 0;
    int tmp = minDim;
    while (tmp >= 2) { tmp >>= 1; ++maxLevels; }
    int lvl = (levels <= 0) ? maxLevels : qMin(levels, maxLevels);
    result.levels = lvl;

    /* 初始化系数为主数据 */
    int cx = m_nx, cy = m_ny, cz = m_nz;
    result.approximation = m_data;
    result.nx = cx; result.ny = cy; result.nz = cz;

    /* 逐层分解 */
    for (int l = 0; l < lvl; ++l) {
        haar3DForward(result);
    }

    /* 分配细节系数(取最后一层的尺寸) */
    int halfX = (result.nx + 1) / 2;
    int halfY = (result.ny + 1) / 2;
    int halfZ = (result.nz + 1) / 2;
    int detailSize = result.approximation.size();

    result.detailX.resize(detailSize, 0.0);
    result.detailY.resize(detailSize, 0.0);
    result.detailZ.resize(detailSize, 0.0);
    result.detailXY.resize(detailSize, 0.0);
    result.detailXZ.resize(detailSize, 0.0);
    result.detailYZ.resize(detailSize, 0.0);

    /* 统计 */
    ++m_stats.totalTransforms;
    m_stats.totalVoxelsProcessed += totalVoxels;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit forwardCompleted(lvl, elapsed);
    return result;
}

// ── 逆变换 ──

/**
 * @brief 执行逆Haar小波变换
 * @param coeffs 小波系数
 * @return 重构的体数据
 */
QVector<double> HaarWavelet3D::inverse(const Coefficients& coeffs)
{
    if (coeffs.approximation.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    Coefficients work = coeffs;

    /* 逐层重构 */
    for (int l = 0; l < coeffs.levels; ++l) {
        haar3DInverse(work);
    }

    ++m_stats.totalInverseTransforms;
    qint64 voxels = static_cast<qint64>(work.nx)
        * work.ny * work.nz;
    m_stats.totalVoxelsProcessed += voxels;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms
            + m_stats.totalInverseTransforms);

    emit inverseCompleted(voxels, elapsed);
    return work.approximation;
}

// ── 阈值去噪 ──

/**
 * @brief 硬阈值去噪: 将绝对值小于threshold的系数置零
 * @param coeffs 系数(原地修改)
 * @param threshold 阈值
 * @return 被置零的系数个数
 */
int HaarWavelet3D::applyHardThreshold(Coefficients& coeffs,
                                       double threshold) const
{
    int count = 0;
    auto applyTo = [&](QVector<double>& v) {
        for (auto& val : v) {
            if (qAbs(val) < threshold) {
                val = 0.0;
                ++count;
            }
        }
    };
    applyTo(coeffs.detailX);
    applyTo(coeffs.detailY);
    applyTo(coeffs.detailZ);
    applyTo(coeffs.detailXY);
    applyTo(coeffs.detailXZ);
    applyTo(coeffs.detailYZ);
    return count;
}

/**
 * @brief 软阈值去噪: 将绝对值小于threshold的系数置零,
 *        其余系数向零收缩threshold
 * @param coeffs 系数(原地修改)
 * @param threshold 阈值
 * @return 被修改的系数个数
 */
int HaarWavelet3D::applySoftThreshold(Coefficients& coeffs,
                                       double threshold) const
{
    int count = 0;
    auto applyTo = [&](QVector<double>& v) {
        for (auto& val : v) {
            double a = qAbs(val);
            if (a < threshold) {
                if (val != 0.0) ++count;
                val = 0.0;
            } else {
                val = (val > 0) ? val - threshold : val + threshold;
                ++count;
            }
        }
    };
    applyTo(coeffs.detailX);
    applyTo(coeffs.detailY);
    applyTo(coeffs.detailZ);
    applyTo(coeffs.detailXY);
    applyTo(coeffs.detailXZ);
    applyTo(coeffs.detailYZ);
    return count;
}

/**
 * @brief 计算各频带能量分布
 * @param coeffs 小波系数
 * @return 7个频带的能量占比(approx,dx,dy,dz,dxy,dxz,dyz)
 */
QVector<double> HaarWavelet3D::energyDistribution(
    const Coefficients& coeffs) const
{
    QVector<double> energies(7, 0.0);
    energies[0] = vectorEnergy(coeffs.approximation);
    energies[1] = vectorEnergy(coeffs.detailX);
    energies[2] = vectorEnergy(coeffs.detailY);
    energies[3] = vectorEnergy(coeffs.detailZ);
    energies[4] = vectorEnergy(coeffs.detailXY);
    energies[5] = vectorEnergy(coeffs.detailXZ);
    energies[6] = vectorEnergy(coeffs.detailYZ);

    double total = 0.0;
    for (double e : energies) total += e;
    if (total > 1e-18) {
        for (auto& e : energies) e /= total;
    }
    return energies;
}

// ── 私有方法 ──

/** @brief 单层1D Haar前向变换(就地) */
void HaarWavelet3D::haar1DForward(QVector<double>& data, int n)
{
    QVector<double> tmp(n, 0.0);
    int half = n / 2;
    double invSqrt2 = 1.0 / qSqrt(2.0);
    for (int i = 0; i < half; ++i) {
        double a = data[2 * i];
        double b = data[2 * i + 1];
        tmp[i] = (a + b) * invSqrt2;
        tmp[half + i] = (a - b) * invSqrt2;
    }
    for (int i = 0; i < n; ++i) {
        data[i] = tmp[i];
    }
}

/** @brief 单层1D Haar逆向变换(就地) */
void HaarWavelet3D::haar1DInverse(QVector<double>& data, int n)
{
    QVector<double> tmp(n, 0.0);
    int half = n / 2;
    double invSqrt2 = 1.0 / qSqrt(2.0);
    for (int i = 0; i < half; ++i) {
        double a = data[i];
        double b = data[half + i];
        tmp[2 * i] = (a + b) * invSqrt2;
        tmp[2 * i + 1] = (a - b) * invSqrt2;
    }
    for (int i = 0; i < n; ++i) {
        data[i] = tmp[i];
    }
}

/** @brief 单层3D Haar前向变换 */
void HaarWavelet3D::haar3DForward(Coefficients& coeffs)
{
    int cx = coeffs.nx, cy = coeffs.ny, cz = coeffs.nz;
    auto& data = coeffs.approximation;

    /* 沿X轴变换 */
    for (int z = 0; z < cz; ++z) {
        for (int y = 0; y < cy; ++y) {
            QVector<double> row(cx);
            for (int x = 0; x < cx; ++x) {
                row[x] = data[x + y * cx + z * cx * cy];
            }
            haar1DForward(row, cx);
            for (int x = 0; x < cx; ++x) {
                data[x + y * cx + z * cx * cy] = row[x];
            }
        }
    }

    /* 沿Y轴变换 */
    for (int z = 0; z < cz; ++z) {
        for (int x = 0; x < cx; ++x) {
            QVector<double> col(cy);
            for (int y = 0; y < cy; ++y) {
                col[y] = data[x + y * cx + z * cx * cy];
            }
            haar1DForward(col, cy);
            for (int y = 0; y < cy; ++y) {
                data[x + y * cx + z * cx * cy] = col[y];
            }
        }
    }

    /* 沿Z轴变换 */
    for (int y = 0; y < cy; ++y) {
        for (int x = 0; x < cx; ++x) {
            QVector<double> depth(cz);
            for (int z = 0; z < cz; ++z) {
                depth[z] = data[x + y * cx + z * cx * cy];
            }
            haar1DForward(depth, cz);
            for (int z = 0; z < cz; ++z) {
                data[x + y * cx + z * cx * cy] = depth[z];
            }
        }
    }
}

/** @brief 单层3D Haar逆向变换 */
void HaarWavelet3D::haar3DInverse(Coefficients& coeffs)
{
    int cx = coeffs.nx, cy = coeffs.ny, cz = coeffs.nz;
    auto& data = coeffs.approximation;

    /* 沿Z轴逆变换 */
    for (int y = 0; y < cy; ++y) {
        for (int x = 0; x < cx; ++x) {
            QVector<double> depth(cz);
            for (int z = 0; z < cz; ++z) {
                depth[z] = data[x + y * cx + z * cx * cy];
            }
            haar1DInverse(depth, cz);
            for (int z = 0; z < cz; ++z) {
                data[x + y * cx + z * cx * cy] = depth[z];
            }
        }
    }

    /* 沿Y轴逆变换 */
    for (int z = 0; z < cz; ++z) {
        for (int x = 0; x < cx; ++x) {
            QVector<double> col(cy);
            for (int y = 0; y < cy; ++y) {
                col[y] = data[x + y * cx + z * cx * cy];
            }
            haar1DInverse(col, cy);
            for (int y = 0; y < cy; ++y) {
                data[x + y * cx + z * cx * cy] = col[y];
            }
        }
    }

    /* 沿X轴逆变换 */
    for (int z = 0; z < cz; ++z) {
        for (int y = 0; y < cy; ++y) {
            QVector<double> row(cx);
            for (int x = 0; x < cx; ++x) {
                row[x] = data[x + y * cx + z * cx * cy];
            }
            haar1DInverse(row, cx);
            for (int x = 0; x < cx; ++x) {
                data[x + y * cx + z * cx * cy] = row[x];
            }
        }
    }
}

/** @brief 向量能量(平方和) */
double HaarWavelet3D::vectorEnergy(const QVector<double>& v)
{
    double sum = 0.0;
    for (double val : v) sum += val * val;
    return sum;
}

/** @brief 重置统计 */
void HaarWavelet3D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
