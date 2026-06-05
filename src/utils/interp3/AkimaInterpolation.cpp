/**
 * @file AkimaInterpolation.cpp
 * @brief Akima 分段三次插值算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/interp3/AkimaInterpolation.h"

#include <QtGlobal>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
AkimaInterpolation::AkimaInterpolation(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 计算 Akima 斜率
 *
 * Akima 公式: 对相邻差商 w[i] 进行加权平均，
 * 权重 |w[i+1] - w[i]| + |w[i-1] - w[i-2]| 避免过冲。
 *
 * @param x 横坐标(严格递增)
 * @param y 纵坐标
 * @return 每个节点处的 Akima 斜率
 */
QVector<double> AkimaInterpolation::computeSlopes(const QVector<double> &x,
                                                  const QVector<double> &y)
{
    const int n = x.size();
    Q_ASSERT(n >= 2);
    // 扩展差商数组: w[-2], w[-1], w[0]..w[n-2], w[n-1], w[n]
    // 用线性外推填充虚拟端点
    QVector<double> w(n + 3, 0.0);
    for (int i = 2; i < n + 1; ++i) {
        w[i] = (y[i - 1] - y[i - 2]) / (x[i - 1] - x[i - 2]);
    }
    // 边界虚拟差商 — Akima 线性外推
    w[1] = 2.0 * w[2] - w[3];
    w[0] = 2.0 * w[1] - w[2];
    w[n + 1] = 2.0 * w[n] - w[n - 1];
    w[n + 2] = 2.0 * w[n + 1] - w[n];

    QVector<double> s(n, 0.0);
    for (int i = 0; i < n; ++i) {
        // 映射: 节点 i 对应差商索引 i+2
        int j = i + 2;
        double aq = std::abs(w[j + 1] - w[j]);
        double ap = std::abs(w[j - 1] - w[j - 2]);
        if (std::abs(aq + ap) < 1e-15) {
            // 均为零时取算术平均
            s[i] = 0.5 * (w[j - 1] + w[j]);
        } else {
            s[i] = (aq * w[j - 1] + ap * w[j]) / (aq + ap);
        }
    }
    return s;
}

/**
 * @brief Hermite 三次基函数求值
 * @param t 归一化参数 [0,1]
 * @param y0 左端值
 * @param y1 右端值
 * @param s0 左端斜率
 * @param s1 右端斜率
 * @param h 段宽度
 * @return 插值结果
 */
double AkimaInterpolation::hermite(double t, double y0, double y1,
                                   double s0, double s1, double h)
{
    // Hermite 基函数
    double h00 = (1.0 + 2.0 * t) * (1.0 - t) * (1.0 - t);
    double h10 = t * (1.0 - t) * (1.0 - t);
    double h01 = t * t * (3.0 - 2.0 * t);
    double h11 = t * t * (t - 1.0);
    return h00 * y0 + h10 * h * s0 + h01 * y1 + h11 * h * s1;
}

/**
 * @brief 执行 Akima 插值
 *
 * 流程:
 * 1. 输入校验(长度一致、单调递增、最少5个点)
 * 2. 计算 Akima 斜率
 * 3. 对每个查询点定位所在段，用 Hermite 三次求值
 * 4. 更新统计并发射信号
 *
 * @param x 控制点横坐标(严格递增)
 * @param y 控制点纵坐标
 * @param xQuery 待查询横坐标数组
 * @return 插值结果数组
 */
QVector<double> AkimaInterpolation::interpolate(const QVector<double> &x,
                                                const QVector<double> &y,
                                                const QVector<double> &xQuery)
{
    m_timer.start();

    QVector<double> result;

    // 输入校验
    if (x.size() != y.size() || x.size() < 5) {
        emit interpolationCompleted(0);
        return result;
    }
    for (int i = 1; i < x.size(); ++i) {
        if (x[i] <= x[i - 1]) {
            emit interpolationCompleted(0);
            return result;
        }
    }

    const int n = x.size();
    const int queryCount = xQuery.size();
    result.resize(queryCount);

    // 计算每个节点的 Akima 斜率
    QVector<double> slopes = computeSlopes(x, y);

    // 对每个查询点定位段并求值
    int seg = 0; // 当前段索引，利用单调性递增扫描
    for (int i = 0; i < queryCount; ++i) {
        double q = xQuery[i];

        // 边界钳位
        if (q <= x[0]) {
            double h = x[1] - x[0];
            result[i] = y[0] + slopes[0] * (q - x[0]);
            continue;
        }
        if (q >= x[n - 1]) {
            double h = x[n - 1] - x[n - 2];
            result[i] = y[n - 1] + slopes[n - 1] * (q - x[n - 1]);
            continue;
        }

        // 定位段: seg 满足 x[seg] <= q < x[seg+1]
        while (seg < n - 2 && x[seg + 1] < q) {
            ++seg;
        }
        // 如果 seg 超过目标则回退
        while (seg > 0 && x[seg] > q) {
            --seg;
        }

        double x0 = x[seg];
        double x1 = x[seg + 1];
        double h = x1 - x0;
        double t = (q - x0) / h;

        result[i] = hermite(t, y[seg], y[seg + 1],
                            slopes[seg], slopes[seg + 1], h);
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalInterpolations++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalInterpolations);

    emit interpolationCompleted(queryCount);
    return result;
}

/**
 * @brief 重置统计计数器
 */
void AkimaInterpolation::resetStatistics()
{
    m_stats.totalInterpolations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
