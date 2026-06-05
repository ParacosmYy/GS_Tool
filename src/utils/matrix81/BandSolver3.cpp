/**
 * @file BandSolver3.cpp
 * @brief 带状线性系统求解器实现 — Thomas算法(LU分解)求解带状矩阵
 */

#include "utils/matrix81/BandSolver3.h"

#include <QElapsedTimer>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
BandSolver3::BandSolver3(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_halfBandwidth(0)
{
}

/**
 * @brief 设置半带宽并执行带状矩阵LU分解(Thomas算法推广)
 *
 * 带状存储格式: bands[i]为第i行的带状数据, 长度为n.
 * 每行以主对角线为中心, 左侧halfBandwidth个元素+对角线+右侧halfBandwidth个元素.
 * 对于边界行, 超出矩阵范围的位置用0填充.
 *
 * 分解算法: Doolittle LU分解, 仅在带宽范围内执行消元,
 * 利用带状稀疏性将复杂度从O(n^3)降至O(n*bw^2).
 *
 * 当对角线元素接近零时执行带宽范围内的部分主元选取.
 *
 * @param bands 带状存储矩阵, bands[i][j]表示第i行中第j列元素
 * @param n 矩阵维度
 * @param halfBandwidth 半带宽(主对角线到最远非零元的距离)
 * @return 分解是否成功(非奇异返回true)
 */
bool BandSolver3::factorize(const QVector<QVector<double>>& bands, int n, int halfBandwidth)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    m_halfBandwidth = halfBandwidth;

    if (n <= 0 || halfBandwidth < 0 || bands.isEmpty()) {
        return false;
    }

    /* 验证输入维度: 每行应有n列(稠密存储) */
    for (int i = 0; i < bands.size(); ++i) {
        if (bands[i].size() < n) return false;
    }

    /* 深拷贝矩阵用于in-place分解 */
    m_factoredBands = bands;

    int diagOffset = 0; /* 稠密存储中列索引就是实际列 */

    /* --- 带状LU分解(Doolittle形式) --- *
     *
     * 对第k步消元:
     *   仅更新第k+1..min(k+bw, n-1)行, 因为更远的行在第k列为零.
     *   每行更新仅影响列 k..min(k+bw, n-1), 因为消元不改变带外零元素.
     *
     * 对于三对角矩阵(bw=1), 退化为经典Thomas算法:
     *   c'[k] = c[k] / b[k]        (仅一个上对角元素)
     *   b[k+1] = b[k+1] - a[k+1]*c'[k]
     */
    for (int k = 0; k < n; ++k) {
        double akk = m_factoredBands[k][k];

        /* 检查主对角线是否过小(接近奇异) */
        if (std::abs(akk) < 1e-15) {
            /* 在带宽范围内执行部分主元选取: 寻找最大对角线元素 */
            int pivotRow = -1;
            double pivotMax = 0.0;
            for (int p = k + 1; p <= std::min(k + halfBandwidth, n - 1); ++p) {
                if (p >= m_factoredBands.size()) break;
                double val = std::abs(m_factoredBands[p][k]);
                if (val > pivotMax) {
                    pivotMax = val;
                    pivotRow = p;
                }
            }

            if (pivotRow < 0 || pivotMax < 1e-15) {
                return false; /* 矩阵在带宽范围内奇异 */
            }

            /* 交换行k和pivotRow */
            for (int j = 0; j < n; ++j) {
                std::swap(m_factoredBands[k][j], m_factoredBands[pivotRow][j]);
            }
            akk = m_factoredBands[k][k];
        }

        /* 消元: 仅处理带宽范围内的行 */
        int rowEnd = std::min(k + halfBandwidth, n - 1);
        for (int i = k + 1; i <= rowEnd; ++i) {
            if (i >= m_factoredBands.size()) break;

            /* 计算乘子 L(i,k) = A(i,k) / A(k,k) */
            double lik = m_factoredBands[i][k] / akk;

            /* 存储L因子到下三角位置 */
            m_factoredBands[i][k] = lik;

            /* 更新第i行的带内元素: A(i,j) -= L(i,k) * A(k,j)
             * 仅更新 j = k+1..min(k+bw, n-1) 范围 */
            int colEnd = std::min(k + halfBandwidth, n - 1);
            for (int j = k + 1; j <= colEnd; ++j) {
                m_factoredBands[i][j] -= lik * m_factoredBands[k][j];
            }
        }
    }

    /* --- 更新统计 --- */
    m_stats.totalFactorizations++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(halfBandwidth, n);
    return true;
}

/**
 * @brief 使用已分解的带状LU因子求解Ax=b(前代+回代)
 *
 * 分解后 A = L*U, 其中L的单位下三角因子存储在m_factoredBands的下三角位置,
 * U的上三角因子存储在上三角位置(含对角线).
 *
 * 前代仅遍历带宽范围内的下三角元素: O(n*bw)
 * 回代仅遍历带宽范围内的上三角元素: O(n*bw)
 *
 * 对于三对角矩阵, 退化为Thomas算法的标准前代/回代:
 *   前代: y[i] = b[i] - a[i]*y[i-1]
 *   回代: x[i] = (y[i] - c[i]*x[i+1]) / d[i]
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> BandSolver3::solve(const QVector<double>& rhs) const
{
    if (m_n == 0 || m_factoredBands.isEmpty()) return {};

    int n = m_n;
    int bw = m_halfBandwidth;

    if (rhs.size() < n) return {};

    QVector<double> x = rhs;

    /* --- 前代: 求解 Ly = b --- *
     * L是单位下三角, L(i,k) = m_factoredBands[i][k] for i > k
     * 由于带状结构, L(i,k)=0 when |i-k| > bw */
    for (int i = 0; i < n; ++i) {
        double sum = x[i];
        int kStart = std::max(0, i - bw);
        for (int k = kStart; k < i; ++k) {
            sum -= m_factoredBands[i][k] * x[k];
        }
        x[i] = sum; /* L对角线为1, 无需除法 */
    }

    /* --- 回代: 求解 Ux = y --- *
     * U是上三角, U(i,i) = m_factoredBands[i][i], U(i,k) = m_factoredBands[i][k] for k > i
     * 带状结构: U(i,k)=0 when |k-i| > bw */
    for (int i = n - 1; i >= 0; --i) {
        double diag = m_factoredBands[i][i];
        if (std::abs(diag) < 1e-15) {
            x[i] = 0.0; /* 奇异: 设为零, 避免除零 */
            continue;
        }

        double sum = x[i];
        int kEnd = std::min(i + bw, n - 1);
        for (int k = i + 1; k <= kEnd; ++k) {
            sum -= m_factoredBands[i][k] * x[k];
        }
        x[i] = sum / diag;
    }

    /* 更新求解统计 */
    m_stats.totalSolves++;
    return x;
}

/**
 * @brief 获取当前半带宽
 * @return 半带宽值
 */
int BandSolver3::halfBandwidth() const
{
    return m_halfBandwidth;
}

/** @brief 重置统计信息 */
void BandSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
