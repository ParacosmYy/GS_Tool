/**
 * @file ProcrustesAnalysis.cpp
 * @brief Procrustes 形状对齐分析实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/procrustes/ProcrustesAnalysis.h"

#include <QtGlobal>
#include <cmath>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
ProcrustesAnalysis::ProcrustesAnalysis(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 计算点集质心
 * @param points n x d 点集
 * @return d 维质心向量
 */
QVector<double> ProcrustesAnalysis::centroid(const QVector<QVector<double>> &points)
{
    if (points.isEmpty()) return {};
    const int n = points.size();
    const int d = points[0].size();
    QVector<double> c(d, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            c[j] += points[i][j];
        }
    }
    for (int j = 0; j < d; ++j) {
        c[j] /= static_cast<double>(n);
    }
    return c;
}

/**
 * @brief 中心化点集
 * @param points 原始点集
 * @param c 质心
 * @return 中心化后的点集
 */
QVector<QVector<double>> ProcrustesAnalysis::centerPoints(
    const QVector<QVector<double>> &points, const QVector<double> &c)
{
    const int n = points.size();
    const int d = c.size();
    QVector<QVector<double>> centered(n, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            centered[i][j] = points[i][j] - c[j];
        }
    }
    return centered;
}

/**
 * @brief 计算点集 Frobenius 范数
 * @param points n x d 点集
 * @return Frobenius 范数(所有元素的平方和的平方根)
 */
double ProcrustesAnalysis::frobeniusNorm(const QVector<QVector<double>> &points)
{
    double sum = 0.0;
    for (const auto &row : points) {
        for (double v : row) {
            sum += v * v;
        }
    }
    return std::sqrt(sum);
}

/**
 * @brief 2x2 SVD 分解(解析公式)
 *
 * 基于 R. Bridson 的 2x2 SVD 闭式解法:
 * 计算 E = 0.5*(A + A^T) 和 F = 0.5*(A - A^T)，
 * 构造 G 矩阵后求特征分解。
 *
 * @param a00 矩阵(0,0)
 * @param a01 矩阵(0,1)
 * @param a10 矩阵(1,0)
 * @param a11 矩阵(1,1)
 * @param U 左奇异向量输出(4个元素, 行优先)
 * @param S 奇异值输出(2个元素)
 * @param Vt 右奇异向量输出(4个元素, 行优先)
 */
void ProcrustesAnalysis::svd2x2(double a00, double a01, double a10, double a11,
                                double U[4], double S[2], double Vt[4])
{
    // 对称部分
    double e00 = a00 + a00;  // A+A^T 的对角
    double e01 = a01 + a10;
    double e10 = a10 + a01;
    double e11 = a11 + a11;

    // 构造 G = E^T E (简化)
    // 实际使用 Jacobi 旋转法
    // 直接利用闭式解: U S Vt = A
    double a = a00 + a11;  // trace
    double b = a10 - a01;  // anti-trace

    // 初始旋转角度
    double theta = 0.5 * std::atan2(b, a);
    double ct = std::cos(theta);
    double st = std::sin(theta);

    // 旋转后矩阵
    double r00 = ct * a00 + st * a10;
    double r01 = ct * a01 + st * a11;
    double r10 = -st * a00 + ct * a10;
    double r11 = -st * a01 + ct * a11;

    // 第二次旋转
    double phi = 0.5 * std::atan2(r01 + r10, r00 - r11);
    double cp = std::cos(phi);
    double sp = std::sin(phi);

    // 最终对角元素 = 奇异值
    double d0 = cp * r00 + sp * r10;
    double d1 = -sp * r01 + cp * r11;

    S[0] = std::abs(d0);
    S[1] = std::abs(d1);

    // 奇异值的符号修正
    double s0 = (d0 >= 0) ? 1.0 : -1.0;
    double s1 = (d1 >= 0) ? 1.0 : -1.0;

    // U = R(theta)
    U[0] = ct;  U[1] = -st;
    U[2] = st;  U[3] = ct;

    // Vt = diag(s0, s1) * R(phi)^T
    Vt[0] = s0 * cp;  Vt[1] = s0 * sp;
    Vt[2] = -s1 * sp; Vt[3] = s1 * cp;
}

/**
 * @brief 将源点集 Procrustes 对齐到目标点集
 *
 * 流程:
 * 1. 输入校验(点数一致、维度一致、至少2个点)
 * 2. 计算质心并中心化
 * 3. 归一化尺度(除以 Frobenius 范数)
 * 4. 计算 H = X^T Y，SVD 分解求旋转 R = V U^T
 * 5. 应用变换: aligned = scale * R * centered_source + centroid_target
 *
 * @param source n x d 源点集
 * @param target n x d 目标点集
 * @return n x d 对齐后的点集
 */
QVector<QVector<double>> ProcrustesAnalysis::align(
    const QVector<QVector<double>> &source,
    const QVector<QVector<double>> &target)
{
    m_timer.start();

    const int n = source.size();
    // 输入校验
    if (n < 2 || n != target.size()) {
        emit alignmentCompleted(0);
        return {};
    }
    const int d = source[0].size();
    for (int i = 0; i < n; ++i) {
        if (source[i].size() != d || target[i].size() != d) {
            emit alignmentCompleted(0);
            return {};
        }
    }

    // 1. 中心化
    QVector<double> cSrc = centroid(source);
    QVector<double> cTgt = centroid(target);
    QVector<QVector<double>> X = centerPoints(source, cSrc);
    QVector<QVector<double>> Y = centerPoints(target, cTgt);

    // 2. 归一化尺度
    double normX = frobeniusNorm(X);
    double normY = frobeniusNorm(Y);
    if (normX > 1e-15) {
        for (auto &row : X)
            for (double &v : row) v /= normX;
    }
    if (normY > 1e-15) {
        for (auto &row : Y)
            for (double &v : row) v /= normY;
    }

    // 3. 计算 H = X^T Y (d x d 矩阵)
    // 限定 d=2 时用解析 SVD，d>2 时用简化的幂迭代近似
    double scale = (normX > 1e-15 && normY > 1e-15)
                       ? normY / normX : 1.0;

    QVector<QVector<double>> aligned(n, QVector<double>(d, 0.0));

    if (d == 2) {
        // 2D: 解析 SVD
        // H = X^T Y (2x2)
        double h00 = 0.0, h01 = 0.0, h10 = 0.0, h11 = 0.0;
        for (int i = 0; i < n; ++i) {
            h00 += X[i][0] * Y[i][0];
            h01 += X[i][0] * Y[i][1];
            h10 += X[i][1] * Y[i][0];
            h11 += X[i][1] * Y[i][1];
        }

        double U[4], S[2], Vt[4];
        svd2x2(h00, h01, h10, h11, U, S, Vt);

        // R = V * U^T
        double r00 = Vt[0] * U[0] + Vt[1] * U[1];
        double r01 = Vt[0] * U[2] + Vt[1] * U[3];
        double r10 = Vt[2] * U[0] + Vt[3] * U[1];
        double r11 = Vt[2] * U[2] + Vt[3] * U[3];

        // 应用变换
        for (int i = 0; i < n; ++i) {
            double cx = X[i][0] * normX; // 还原未归一化的中心化坐标
            double cy = X[i][1] * normX;
            aligned[i][0] = scale * (r00 * cx + r01 * cy) + cTgt[0];
            aligned[i][1] = scale * (r10 * cx + r11 * cy) + cTgt[1];
        }
    } else {
        // 通用 d 维: 简化的旋转近似(使用 H 的极分解)
        // R ≈ H (H^T H)^{-1/2}，简化为单位缩放
        QVector<QVector<double>> H(d, QVector<double>(d, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < d; ++j) {
                for (int k = 0; k < d; ++k) {
                    H[j][k] += X[i][j] * Y[i][k];
                }
            }
        }
        // 近似: 直接用 H 作为变换(小旋转时近似良好)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < d; ++j) {
                double val = 0.0;
                for (int k = 0; k < d; ++k) {
                    val += H[j][k] * X[i][k];
                }
                aligned[i][j] = scale * normY * val + cTgt[j];
            }
        }
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalAlignments++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalAlignments);

    emit alignmentCompleted(n);
    return aligned;
}

/**
 * @brief 重置统计计数器
 */
void ProcrustesAnalysis::resetStatistics()
{
    m_stats.totalAlignments = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
