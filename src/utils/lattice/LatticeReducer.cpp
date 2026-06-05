/**
 * @file LatticeReducer.cpp
 * @brief LLL格基规约算法实现
 */

#include "utils/lattice/LatticeReducer.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LatticeReducer::LatticeReducer(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置LLL参数delta @param delta 松弛参数 */
void LatticeReducer::setDelta(double delta)
{
    m_delta = qBound(0.25, delta, 1.0);
}

/** @brief 设置收敛精度 @param epsilon 数值精度 */
void LatticeReducer::setEpsilon(double epsilon)
{
    m_epsilon = qMax(1e-15, epsilon);
}

/** @brief 设置最大规约步数 @param maxSteps 最大步数 */
void LatticeReducer::setMaxSteps(int maxSteps)
{
    m_maxSteps = qMax(0, maxSteps);
}

/** @brief 执行LLL规约
 *  @param basis 输入基向量
 *  @return 规约结果 */
LatticeReducer::ReductionResult LatticeReducer::reduce(
    const QVector<QVector<double>>& basis)
{
    QElapsedTimer timer;
    timer.start();

    ReductionResult result;
    result.success = false;

    if (!validateBasis(basis)) {
        emit reductionCompleted(0, 0);
        return result;
    }

    int n = basis.size();
    int dim = basis[0].size();

    /* 复制基向量用于操作 */
    QVector<QVector<double>> B = basis;

    /* 计算Gram-Schmidt正交化 */
    auto gsResult = gramSchmidt(B);
    QVector<QVector<double>> Bstar = gsResult.first;
    QVector<QVector<double>> mu = gsResult.second;

    int k = 1;
    int steps = 0;

    while (k < n) {
        bool exceeded = (m_maxSteps > 0 && steps >= m_maxSteps);

        /* Size-reduce B[k] */
        for (int j = k - 1; j >= 0; --j) {
            if (qAbs(mu[k][j]) > 0.5) {
                double coeff = nearestInt(mu[k][j]);
                /* B[k] = B[k] - coeff * B[j] */
                for (int d = 0; d < dim; ++d) {
                    B[k][d] -= coeff * B[j][d];
                }
                /* 更新μ */
                for (int i = 0; i <= j; ++i) {
                    mu[k][i] -= coeff * mu[j][i];
                }
                ++m_stats.totalVectorOps;
            }
        }

        /* Lovasz条件检查 */
        double BstarKNorm = vectorNorm(Bstar[k]);
        double BstarK1Norm = vectorNorm(Bstar[k - 1]);

        if (BstarK1Norm < m_epsilon) break;

        double lovaszLHS = BstarKNorm * BstarKNorm;
        double lovaszRHS = (m_delta - mu[k][k - 1] * mu[k][k - 1])
                           * BstarK1Norm * BstarK1Norm;

        if (lovaszLHS >= lovaszRHS) {
            /* 满足Lovasz条件，继续 */
            ++k;
        } else {
            /* 交换B[k]和B[k-1] */
            std::swap(B[k], B[k - 1]);

            /* 重新计算Gram-Schmidt */
            gsResult = gramSchmidt(B);
            Bstar = gsResult.first;
            mu = gsResult.second;

            ++result.swapCount;
            ++m_stats.totalSwaps;

            emit basisSwap(k);

            k = qMax(k - 1, 1);
        }

        ++steps;
        ++result.reductionSteps;
        ++m_stats.totalVectorOps;

        if (exceeded) break;
    }

    result.reducedBasis = B;
    result.success = true;

    /* 计算各基向量范数 */
    result.basisNorms.resize(n);
    for (int i = 0; i < n; ++i) {
        result.basisNorms[i] = vectorNorm(B[i]);
    }

    result.originalDeterminant = determinant(basis);
    result.reducedDeterminant = determinant(B);

    ++m_stats.totalReductions;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalReductions);

    emit reductionCompleted(result.swapCount, result.reductionSteps);
    return result;
}

/** @brief Gram-Schmidt正交化
 *  @param basis 输入基
 *  @return (正交化基, 投影系数μ) */
QPair<QVector<QVector<double>>, QVector<QVector<double>>>
LatticeReducer::gramSchmidt(const QVector<QVector<double>>& basis) const
{
    int n = basis.size();
    if (n == 0) return {{}, {}};

    int dim = basis[0].size();
    QVector<QVector<double>> Bstar(n, QVector<double>(dim, 0.0));
    QVector<QVector<double>> mu(n, QVector<double>(n, 0.0));

    /* 第一个向量 */
    Bstar[0] = basis[0];

    for (int i = 1; i < n; ++i) {
        Bstar[i] = basis[i];

        for (int j = 0; j < i; ++j) {
            double BstarJNorm2 = dotProduct(Bstar[j], Bstar[j]);
            if (BstarJNorm2 > m_epsilon) {
                mu[i][j] = dotProduct(basis[i], Bstar[j]) / BstarJNorm2;
            } else {
                mu[i][j] = 0.0;
            }

            /* 减去投影 */
            for (int d = 0; d < dim; ++d) {
                Bstar[i][d] -= mu[i][j] * Bstar[j][d];
            }
        }
    }

    return {Bstar, mu};
}

/** @brief 计算格的行列式(近似)
 *  @param basis 基向量
 *  @return 行列式绝对值 */
double LatticeReducer::determinant(
    const QVector<QVector<double>>& basis) const
{
    if (basis.isEmpty()) return 0.0;

    /* 使用Gram-Schmidt正交基的范数乘积 */
    auto gsResult = gramSchmidt(basis);
    const auto& Bstar = gsResult.first;

    double det = 1.0;
    for (const auto& v : Bstar) {
        det *= dotProduct(v, v);
    }
    return qSqrt(qAbs(det));
}

/** @brief 计算向量范数 */
double LatticeReducer::vectorNorm(const QVector<double>& v)
{
    return qSqrt(dotProduct(v, v));
}

/** @brief 计算两个向量的内积 */
double LatticeReducer::dotProduct(const QVector<double>& a,
                                  const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/** @brief 计算最短向量近似解(SVP)
 *  @param basis 格基
 *  @return 最短向量的范数 */
double LatticeReducer::shortestVectorApprox(
    const QVector<QVector<double>>& basis)
{
    ReductionResult result = reduce(basis);
    if (!result.success || result.basisNorms.isEmpty()) {
        return 0.0;
    }
    return result.basisNorms[0];
}

/** @brief 获取统计信息 */
LatticeReducer::Stats LatticeReducer::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void LatticeReducer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 向量减法 */
QVector<double> LatticeReducer::vecSub(const QVector<double>& a,
                                       const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = a[i] - b[i];
    }
    return result;
}

/** @brief 向量数乘 */
QVector<double> LatticeReducer::vecScale(const QVector<double>& v, double s)
{
    QVector<double> result(v.size());
    for (int i = 0; i < v.size(); ++i) {
        result[i] = v[i] * s;
    }
    return result;
}

/** @brief 取整 */
double LatticeReducer::nearestInt(double x)
{
    return qFloor(x + 0.5);
}

/** @brief 验证基的有效性 */
bool LatticeReducer::validateBasis(
    const QVector<QVector<double>>& basis) const
{
    if (basis.isEmpty()) return false;

    int dim = basis[0].size();
    if (dim == 0) return false;

    for (const auto& v : basis) {
        if (v.size() != dim) return false;
    }
    return true;
}
