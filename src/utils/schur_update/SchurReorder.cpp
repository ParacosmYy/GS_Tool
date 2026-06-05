/**
 * @file SchurReorder.cpp
 * @brief Schur形式重排实现 — 隐式QR步交换对角块
 *
 * 通过相邻对角块交换(bubble sort风格)将selected中指定的
 * 特征值逐步移到左上角。支持1x1和2x2对角块。
 */

#include "utils/schur_update/SchurReorder.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

SchurReorder::SchurReorder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool SchurReorder::isTwoByTwoBlock(const QVector<QVector<double>>& T,
                                   int i) const
{
    int n = T.size();
    if (i + 1 >= n) return false;
    return std::abs(T[i + 1][i]) > 1e-14;
}

void SchurReorder::swapBlocks(QVector<QVector<double>>& T,
                              QVector<QVector<double>>& Q,
                              int p, int q)
{
    int n = T.size();
    if (p == q) return;

    /* 使用冒泡交换逐步移动 */
    int dir = (q > p) ? 1 : -1;
    for (int k = p; k != q; k += dir) {
        int i = k;
        int j = k + dir;
        if (j < 0 || j >= n) continue;

        /* 提取2x2子块用于构造旋转 */
        double tii = T[i][i];
        double tij = (j > i) ? T[i][j] : T[i][i]; // 用于特征值计算
        double tji = (j > i) ? T[j][i] : T[i][j];
        double tjj = T[j][j];

        /* 构造Givens旋转消除子对角元素实现块交换 */
        double a11 = tii - tjj;
        double a21 = (j > i) ? tji : -tji;
        double denom = std::sqrt(a11 * a11 + a21 * a21);

        if (denom < 1e-15) continue;

        double c = a11 / denom;
        double s = a21 / denom;

        /* 旋转T的行i和行j (从min(i,j)列开始) */
        int startCol = (i < j) ? i : j;
        if (startCol > 0) startCol -= 1;
        for (int col = startCol; col < n; ++col) {
            double ri = T[i][col], rj = T[j][col];
            T[i][col] = c * ri + s * rj;
            T[j][col] = -s * ri + c * rj;
        }

        /* 旋转T的列i和列j */
        int endRow = (i > j) ? i + 1 : j + 1;
        if (endRow > n) endRow = n;
        for (int row = 0; row < endRow; ++row) {
            double ci = T[row][i], cj = T[row][j];
            T[row][i] = c * ci + s * cj;
            T[row][j] = -s * ci + c * cj;
        }

        /* 旋转Q的列i和列j */
        for (int row = 0; row < n; ++row) {
            double qi = Q[row][i], qj = Q[row][j];
            Q[row][i] = c * qi + s * qj;
            Q[row][j] = -s * qi + c * qj;
        }
    }
}

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
SchurReorder::reorder(const QVector<QVector<double>>& T,
                      const QVector<QVector<double>>& Q,
                      const QVector<int>& selected)
{
    QElapsedTimer timer;
    timer.start();

    int n = T.size();
    if (n == 0) return {{}, {}};

    /* 复制T和Q用于就地修改 */
    QVector<QVector<double>> Tnew = T;
    QVector<QVector<double>> Qnew = Q;

    /* 标记哪些位置需要移到顶部 */
    QVector<bool> isSelected(n, false);
    for (int idx : selected) {
        if (idx >= 0 && idx < n) isSelected[idx] = true;
    }

    /* 冒泡排序: 将选中的块逐步交换到左上角 */
    int targetPos = 0;
    for (int pass = 0; pass < n; ++pass) {
        for (int i = targetPos; i < n; ++i) {
            if (isSelected[i]) {
                if (i != targetPos) {
                    swapBlocks(Tnew, Qnew, i, targetPos);
                    /* 更新isSelected标记跟随交换 */
                    std::swap(isSelected[i], isSelected[targetPos]);
                }
                targetPos++;
                break;
            }
        }
        if (targetPos >= static_cast<int>(selected.size())) break;
    }

    /* 清理数值噪声: 将应该是零的元素置零 */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i; ++j)
            if (j < i - 1 && std::abs(Tnew[i][j]) < 1e-12)
                Tnew[i][j] = 0.0;

    m_stats.totalReorders++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReorders;

    emit reorderCompleted(n);
    return {Tnew, Qnew};
}

void SchurReorder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
