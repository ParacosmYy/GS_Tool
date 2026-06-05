/**
 * @file KullbackLeibler.cpp
 * @brief KL散度与交叉熵计算器实现 — 分布差异度量
 */

#include "utils/entropy3/KullbackLeibler.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
KullbackLeibler::KullbackLeibler(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算KL散度 D_KL(P||Q) @param p 分布P @param q 分布Q @return KL散度 */
double KullbackLeibler::klDivergence(const QVector<double>& p,
                                     const QVector<double>& q) const
{
    m_timer.start();

    if (p.size() != q.size() || p.isEmpty()) {
        return qQNaN();
    }

    QVector<double> pn = normalize(p);
    QVector<double> qn = normalize(q);

    double kl = 0.0;
    for (int i = 0; i < pn.size(); ++i) {
        if (pn[i] > 0.0 && qn[i] > 0.0) {
            kl += pn[i] * qLn(pn[i] / qn[i]) / qLn(2.0);
        } else if (pn[i] > 0.0 && qn[i] <= 0.0) {
            /* Q为0但P非0, KL散度无界 */
            kl = 1e10;
            break;
        }
        /* P(i)=0 时贡献为0，跳过 */
    }

    ++m_stats.totalComputations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return kl;
}

/** @brief 计算交叉熵 H(P,Q) @param p 真实分布 @param q 近似分布 @return 交叉熵 */
double KullbackLeibler::crossEntropy(const QVector<double>& p,
                                     const QVector<double>& q) const
{
    m_timer.start();

    if (p.size() != q.size() || p.isEmpty()) {
        return qQNaN();
    }

    QVector<double> pn = normalize(p);
    QVector<double> qn = normalize(q);

    double h = 0.0;
    for (int i = 0; i < pn.size(); ++i) {
        if (pn[i] > 0.0 && qn[i] > 0.0) {
            h -= pn[i] * qLn(qn[i]) / qLn(2.0);
        } else if (pn[i] > 0.0 && qn[i] <= 0.0) {
            h = 1e10;
            break;
        }
    }

    ++m_stats.totalComputations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return h;
}

/** @brief 计算JS散度 @param p 分布P @param q 分布Q @return JS散度 */
double KullbackLeibler::jsDivergence(const QVector<double>& p,
                                     const QVector<double>& q) const
{
    m_timer.start();

    if (p.size() != q.size() || p.isEmpty()) {
        return qQNaN();
    }

    QVector<double> pn = normalize(p);
    QVector<double> qn = normalize(q);

    /* M = (P + Q) / 2 */
    QVector<double> m;
    m.reserve(pn.size());
    for (int i = 0; i < pn.size(); ++i) {
        m.append((pn[i] + qn[i]) / 2.0);
    }

    /* JS(P||Q) = 0.5 * KL(P||M) + 0.5 * KL(Q||M) */
    double klPM = 0.0;
    double klQM = 0.0;
    for (int i = 0; i < pn.size(); ++i) {
        if (pn[i] > 0.0 && m[i] > 0.0) {
            klPM += pn[i] * qLn(pn[i] / m[i]) / qLn(2.0);
        }
        if (qn[i] > 0.0 && m[i] > 0.0) {
            klQM += qn[i] * qLn(qn[i] / m[i]) / qLn(2.0);
        }
    }

    double js = 0.5 * klPM + 0.5 * klQM;

    ++m_stats.totalComputations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return js;
}

/** @brief 计算Shannon熵 @param p 概率分布 @return Shannon熵(bits) */
double KullbackLeibler::shannonEntropy(const QVector<double>& p) const
{
    m_timer.start();

    QVector<double> pn = normalize(p);

    double h = 0.0;
    for (double pi : pn) {
        if (pi > 0.0) {
            h -= pi * qLn(pi) / qLn(2.0);
        }
    }

    ++m_stats.totalComputations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return h;
}

/** @brief 重置统计 */
void KullbackLeibler::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

/** @brief 归一化概率向量 @param v 输入向量 @return 归一化向量 */
QVector<double> KullbackLeibler::normalize(const QVector<double>& v)
{
    double sum = 0.0;
    for (double val : v) {
        sum += qMax(0.0, val); /* 忽略负值 */
    }
    if (qFuzzyIsNull(sum)) {
        return v; /* 无法归一化则原样返回 */
    }

    QVector<double> result;
    result.reserve(v.size());
    for (double val : v) {
        result.append(qMax(0.0, val) / sum);
    }
    return result;
}

/** @brief 安全对数 @param x 输入 @param base 底 @return 对数值 */
double KullbackLeibler::safeLog(double x, double base)
{
    if (x <= 0.0) return 0.0;
    return qLn(x) / qLn(base);
}
