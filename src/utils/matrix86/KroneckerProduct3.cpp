#include "KroneckerProduct3.h"
#include <QElapsedTimer>

/**
 * @brief 构造函数，初始化Kronecker积计算器
 * @param parent 父QObject对象指针
 */
KroneckerProduct3::KroneckerProduct3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算两个矩阵的Kronecker积
 *
 * Kronecker积定义：对于A(m x n)和B(p x q)，
 * C = A ⊗ B 是一个(mp x nq)矩阵，其中
 * C[i*p+j, k*q+l] = A[i,k] * B[j,l]
 *
 * 结果矩阵的每个块都是A的对应元素乘以整个B矩阵。
 *
 * @param a 左操作数矩阵A(m x n)
 * @param b 右操作数矩阵B(p x q)
 * @return Kronecker积矩阵(mp x nq)
 */
QVector<QVector<double>> KroneckerProduct3::compute(const QVector<QVector<double>>& a,
                                                      const QVector<QVector<double>>& b)
{
    QElapsedTimer timer;
    timer.start();

    const int rowsA = a.size();
    const int colsA = (rowsA > 0) ? a[0].size() : 0;
    const int rowsB = b.size();
    const int colsB = (rowsB > 0) ? b[0].size() : 0;

    if (rowsA == 0 || colsA == 0 || rowsB == 0 || colsB == 0) return {};

    const int resultRows = rowsA * rowsB;
    const int resultCols = colsA * colsB;

    /// 初始化结果矩阵
    QVector<QVector<double>> result(resultRows, QVector<double>(resultCols, 0.0));

    /// 计算Kronecker积
    long long elementsComputed = 0;
    for (int ia = 0; ia < rowsA; ++ia) {
        for (int ja = 0; ja < colsA; ++ja) {
            double aVal = a[ia][ja];
            for (int ib = 0; ib < rowsB; ++ib) {
                for (int jb = 0; jb < colsB; ++jb) {
                    result[ia * rowsB + ib][ja * colsB + jb] = aVal * b[ib][jb];
                    ++elementsComputed;
                }
            }
        }
    }

    /// 更新统计信息
    m_stats.totalProducts++;
    m_stats.totalElementsComputed += static_cast<int>(elementsComputed);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProducts;

    emit productComputed(rowsA, colsA, rowsB, colsB);
    return result;
}

/**
 * @brief 向量形式的Kronecker积（展平输出）
 *
 * 将两个向量视为列向量，计算Kronecker积并展平为一维向量。
 * a(m) ⊗ b(n) = [a[0]*b, a[1]*b, ..., a[m-1]*b]，长度为m*n。
 *
 * @param a 左操作数向量
 * @param b 右操作数向量
 * @return 展平后的Kronecker积向量
 */
QVector<double> KroneckerProduct3::computeFlat(const QVector<double>& a, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    const int na = a.size();
    const int nb = b.size();

    if (na == 0 || nb == 0) return {};

    QVector<double> result;
    result.reserve(na * nb);

    /// 展平Kronecker积
    for (int i = 0; i < na; ++i) {
        for (int j = 0; j < nb; ++j) {
            result.append(a[i] * b[j]);
        }
    }

    /// 更新统计信息
    m_stats.totalProducts++;
    m_stats.totalElementsComputed += na * nb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProducts;

    return result;
}

/**
 * @brief 获取当前统计数据
 * @return 包含计算次数、元素数和平均耗时的Stats结构
 */
KroneckerProduct3::Stats KroneckerProduct3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void KroneckerProduct3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
