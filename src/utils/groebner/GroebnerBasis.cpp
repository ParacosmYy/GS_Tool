/**
 * @file GroebnerBasis.cpp
 * @brief Groebner基实现 — Buchberger算法(单变量)
 */

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

#include "utils/groebner/GroebnerBasis.h"

GroebnerBasis::GroebnerBasis(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

QVector<QVector<double>> GroebnerBasis::compute(
    const QVector<QVector<double>>& polynomials)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> basis;

    /* 过滤零多项式并首一化 */
    for (const auto& p : polynomials) {
        QVector<double> sp = strip(p);
        if (!sp.isEmpty())
            basis.append(monic(sp));
    }

    if (basis.isEmpty()) {
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(0);
        return basis;
    }

    /* 对于单变量多项式，Groebner基就是所有多项式的GCD */
    /* Buchberger算法: 计算所有S-多项式的归约 */
    bool changed = true;
    int maxIter = 500; /* 防止无限循环 */
    int iter = 0;

    while (changed && iter < maxIter) {
        changed = false;
        ++iter;

        QVector<QVector<double>> newBasis = basis;

        for (int i = 0; i < basis.size() && !changed; ++i) {
            for (int j = i + 1; j < basis.size() && !changed; ++j) {
                QVector<double> s = sPolynomial(basis[i], basis[j]);
                s = strip(s);

                if (s.isEmpty())
                    continue;

                QVector<double> r = reduce(s, newBasis);
                r = strip(r);

                if (!r.isEmpty()) {
                    newBasis.append(monic(r));
                    changed = true;
                }
            }
        }

        basis = newBasis;
    }

    /* 化简基: 用基中其他多项式归约每个基元素 */
    QVector<QVector<double>> reduced;
    for (int i = 0; i < basis.size(); ++i) {
        QVector<double> temp = basis[i];
        for (int j = 0; j < reduced.size(); ++j)
            temp = polyMod(temp, reduced[j]);
        temp = strip(temp);
        if (!temp.isEmpty())
            reduced.append(monic(temp));
    }

    basis = reduced;

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalComputations, 1ULL);

    emit computationCompleted(basis.size());
    return basis;
}

QVector<double> GroebnerBasis::sPolynomial(const QVector<double>& f,
                                             const QVector<double>& g)
{
    /* 单变量S-多项式: S(f,g) = LT(g)*f - LT(f)*g */
    /* LT = 首项 */
    int degF = static_cast<int>(f.size()) - 1;
    int degG = static_cast<int>(g.size()) - 1;

    if (degF < 0 || degG < 0)
        return {};

    /* 单变量情况下，LCM(LM(f), LM(g)) = x^max(degF, degG) */
    /* S(f,g) = x^(max-degF)*f * lc(g) - x^(max-degG)*g * lc(f) */
    int maxDeg = qMax(degF, degG);
    double lcF = f[0];
    double lcG = g[0];

    if (std::abs(lcF) < 1e-15 || std::abs(lcG) < 1e-15)
        return {};

    /* 构造 x^(max-degF)*f * lcG */
    int sizeF = maxDeg + 1;
    QVector<double> termF(sizeF, 0.0);
    int offsetF = maxDeg - degF;
    for (int i = 0; i <= degF; ++i)
        termF[i + offsetF] = f[i] * lcG;

    /* 构造 x^(max-degG)*g * lcF */
    int sizeG = maxDeg + 1;
    QVector<double> termG(sizeG, 0.0);
    int offsetG = maxDeg - degG;
    for (int i = 0; i <= degG; ++i)
        termG[i + offsetG] = g[i] * lcF;

    /* 相减 */
    int sizeResult = qMax(sizeF, sizeG);
    QVector<double> result(sizeResult, 0.0);
    for (int i = 0; i < sizeResult; ++i) {
        double a = (i < sizeF) ? termF[i] : 0.0;
        double b = (i < sizeG) ? termG[i] : 0.0;
        result[i] = a - b;
    }

    return strip(result);
}

QVector<double> GroebnerBasis::reduce(QVector<double> p,
                                       const QVector<QVector<double>>& basis)
{
    /* 反复用基中的多项式进行归约 */
    bool changed = true;
    int safety = 1000;

    while (changed && safety > 0) {
        changed = false;
        --safety;

        p = strip(p);
        if (p.isEmpty())
            break;

        int degP = static_cast<int>(p.size()) - 1;

        for (const auto& b : basis) {
            int degB = static_cast<int>(b.size()) - 1;
            if (degB < 0 || degB > degP)
                continue;

            if (std::abs(b[0]) < 1e-15)
                continue;

            /* 尝试消除p的首项 */
            if (degP >= degB) {
                double coeff = p[0] / b[0];
                int shift = degP - degB;
                for (int i = 0; i <= degB; ++i)
                    p[i + shift] -= coeff * b[i];
                changed = true;
                break;
            }
        }
    }

    return strip(p);
}

QVector<double> GroebnerBasis::polyMod(const QVector<double>& a,
                                        const QVector<double>& b)
{
    QVector<double> result = a;
    int degB = static_cast<int>(b.size()) - 1;

    if (degB < 0 || std::abs(b[0]) < 1e-15)
        return result;

    while (true) {
        result = strip(result);
        int degR = static_cast<int>(result.size()) - 1;
        if (degR < degB)
            break;

        double coeff = result[0] / b[0];
        int shift = degR - degB;
        for (int i = 0; i <= degB; ++i)
            result[i + shift] -= coeff * b[i];
    }

    return strip(result);
}

QVector<double> GroebnerBasis::strip(const QVector<double>& p)
{
    int start = 0;
    while (start < p.size() - 1 && std::abs(p[start]) < 1e-15)
        ++start;
    return QVector<double>(p.begin() + start, p.end());
}

QVector<double> GroebnerBasis::scale(const QVector<double>& p, double s)
{
    QVector<double> result = p;
    for (auto& c : result)
        c *= s;
    return result;
}

QVector<double> GroebnerBasis::monic(const QVector<double>& p)
{
    if (p.isEmpty() || std::abs(p[0]) < 1e-15)
        return p;
    return scale(p, 1.0 / p[0]);
}

void GroebnerBasis::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
