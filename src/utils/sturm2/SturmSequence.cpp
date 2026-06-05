/**
 * @file SturmSequence.cpp
 * @brief Sturm序列多项式实根隔离实现
 */

#include "utils/sturm2/SturmSequence.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
SturmChain::SturmChain(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 多项式除法取余式
 * @param dividend 被除式系数(从高次到低次)
 * @param divisor 除式系数(从高次到低次)
 * @return 余式系数(取负，用于Sturm序列)
 */
QVector<double> SturmChain::polyRemainder(
    const QVector<double>& dividend,
    const QVector<double>& divisor)
{
    if (divisor.isEmpty()) return dividend;

    int degD = dividend.size() - 1;
    int degS = divisor.size() - 1;
    if (degD < degS) return dividend;

    QVector<double> rem = dividend;
    double leadCoeff = divisor[0];
    if (std::abs(leadCoeff) < 1e-15) return dividend;

    for (int i = 0; i <= degD - degS; ++i) {
        double factor = rem[i] / leadCoeff;
        for (int j = 0; j <= degS; ++j) {
            rem[i + j] -= factor * divisor[j];
        }
    }

    /* 提取余式部分(低degS项) */
    QVector<double> result;
    int start = degD - degS + 1;
    for (int i = start; i <= degD; ++i) {
        result.append(rem[i]);
    }

    /* 移除前导零 */
    while (result.size() > 1 && std::abs(result[0]) < 1e-15) {
        result.removeFirst();
    }

    /* Sturm序列需要取负 */
    for (auto& v : result) v = -v;

    return result;
}

/**
 * @brief 构造Sturm序列
 * @param coeffs 多项式系数(从高次到低次)
 * @return Sturm序列(多项式列表)
 *
 * p0 = P(x), p1 = P'(x), pi = -rem(pi-2, pi-1)
 */
QVector<QVector<double>> SturmChain::buildSequence(
    const QVector<double>& coeffs)
{
    QVector<QVector<double>> seq;
    if (coeffs.size() < 2) return seq;

    /* p0 = P(x) */
    seq.append(coeffs);

    /* p1 = P'(x) — 形式导数 */
    int deg = coeffs.size() - 1;
    QVector<double> deriv;
    for (int i = 0; i < deg; ++i) {
        deriv.append(coeffs[i] * (deg - i));
    }
    if (deriv.isEmpty()) return seq;
    seq.append(deriv);

    /* pi = -rem(pi-2, pi-1) */
    while (seq.last().size() > 1) {
        auto rem = polyRemainder(seq[seq.size() - 2], seq[seq.size() - 1]);
        if (rem.isEmpty() || rem.size() < 1) break;
        seq.append(rem);
    }

    return seq;
}

/**
 * @brief 计算多项式在x处的值(Horner法则)
 * @param coeffs 多项式系数
 * @param x 求值点
 * @return 多项式值
 */
double SturmChain::polyEval(const QVector<double>& coeffs, double x)
{
    if (coeffs.isEmpty()) return 0.0;

    double result = coeffs[0];
    for (int i = 1; i < coeffs.size(); ++i) {
        result = result * x + coeffs[i];
    }
    return result;
}

/**
 * @brief 计算Sturm序列在x处的符号变化数
 * @param sequence Sturm序列
 * @param x 求值点
 * @return 符号变化次数
 */
int SturmChain::signChanges(const QVector<QVector<double>>& sequence,
                                double x)
{
    int changes = 0;
    int prevSign = 0;

    for (const auto& poly : sequence) {
        double val = polyEval(poly, x);
        int sign = 0;
        if (val > 1e-15) sign = 1;
        else if (val < -1e-15) sign = -1;

        if (sign != 0) {
            if (prevSign != 0 && sign != prevSign) {
                ++changes;
            }
            prevSign = sign;
        }
    }

    return changes;
}

/**
 * @brief 计算多项式在区间[a,b]内的实根个数
 * @param coeffs 多项式系数(从高次到低次)
 * @param a 区间左端点
 * @param b 区间右端点
 * @return 实根个数
 *
 * 根据Sturm定理: V(a) - V(b) = 区间[a,b]内的不同实根个数。
 */
int SturmChain::countRoots(const QVector<double>& coeffs,
                               double a, double b)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.size() < 2) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalIsolated;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalIsolated;
        return 0;
    }

    auto seq = buildSequence(coeffs);
    if (seq.isEmpty()) return 0;

    int count = signChanges(seq, a) - signChanges(seq, b);
    count = qMax(0, count);

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalIsolated;
    m_stats.totalRoots += static_cast<quint64>(count);
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalIsolated;

    return count;
}

/**
 * @brief 隔离多项式在[lo,hi]内所有实根的区间
 * @param coeffs 多项式系数(从高次到低次)
 * @param lo 搜索下界
 * @param hi 搜索上界
 * @return 各根的隔离区间列表
 *
 * 使用递归二分法: 在每个子区间内计数根，若恰有1个根则记录，
 * 若有多个根则继续二分。
 */
QVector<QPair<double, double>> SturmChain::isolateRoots(
    const QVector<double>& coeffs, double lo, double hi)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> intervals;

    if (coeffs.size() < 2) {
        emit isolated(0);
        return intervals;
    }

    auto seq = buildSequence(coeffs);
    if (seq.isEmpty()) {
        emit isolated(0);
        return intervals;
    }

    int totalRoots = signChanges(seq, lo) - signChanges(seq, hi);
    totalRoots = qMax(0, totalRoots);

    if (totalRoots == 0) {
        emit isolated(0);
        return intervals;
    }

    /* 二分递归隔离 */
    QVector<QPair<double, double>> stack;
    stack.append({lo, hi});

    while (!stack.isEmpty() && intervals.size() < totalRoots) {
        auto range = stack.takeLast();
        double a = range.first;
        double b = range.second;

        int cnt = signChanges(seq, a) - signChanges(seq, b);
        cnt = qMax(0, cnt);

        if (cnt == 0) continue;

        if (cnt == 1) {
            intervals.append({a, b});
            continue;
        }

        /* 多个根，二分 */
        double mid = (a + b) / 2.0;
        if (mid <= a || mid >= b) {
            /* 区间太小，作为单个根区间 */
            intervals.append({a, b});
            continue;
        }

        stack.append({a, mid});
        stack.append({mid, b});
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalIsolated;
    m_stats.totalRoots += static_cast<quint64>(intervals.size());
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalIsolated;

    emit isolated(intervals.size());
    return intervals;
}

/** @brief 重置统计信息 */
void SturmChain::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
