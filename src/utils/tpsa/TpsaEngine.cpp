/**
 * @file TpsaEngine.cpp
 * @brief TPSA引擎实现 — 截断幂级数自动微分
 */

#include "utils/tpsa/TpsaEngine.h"

#include <QElapsedTimer>

#include <algorithm>
#include <numeric>

/* ==================== Tpsa 内联方法 ==================== */

/** @brief 获取常数部分 @return 常数项值 */
double TpsaEngine::Tpsa::constant() const
{
    MonoIndex zero(numVars, 0);
    auto it = terms.find(zero);
    return (it != terms.end()) ? it->second : 0.0;
}

/** @brief 对变量v的偏导数 @param v 变量索引 @return 偏导TPSA */
TpsaEngine::Tpsa TpsaEngine::Tpsa::derivative(int v) const
{
    if (v < 0 || v >= numVars) return *this;

    Tpsa result;
    result.order = order;
    result.numVars = numVars;

    for (const auto& [mono, coeff] : terms) {
        if (mono[v] == 0) continue;
        MonoIndex newMono = mono;
        double newCoeff = coeff * mono[v];
        --newMono[v];
        result.terms[newMono] += newCoeff;
    }
    return result;
}

/** @brief 评估(代入变量值) @param values 变量值 @return 函数值 */
double TpsaEngine::Tpsa::evaluate(const QVector<double>& values) const
{
    double result = 0.0;
    for (const auto& [mono, coeff] : terms) {
        double termVal = coeff;
        for (int i = 0; i < numVars && i < values.size(); ++i) {
            if (mono[i] != 0) {
                termVal *= std::pow(values[i], mono[i]);
            }
        }
        result += termVal;
    }
    return result;
}

/** @brief 项数 @return 非零项数 */
int TpsaEngine::Tpsa::termCount() const
{
    return static_cast<int>(terms.size());
}

/* ==================== TpsaEngine ==================== */

/** @brief 构造函数 @param numVars 变量数 @param maxOrder 最大阶数 @param parent 父对象 */
TpsaEngine::TpsaEngine(int numVars, int maxOrder, QObject* parent)
    : QObject(parent)
    , m_numVars(std::max(1, numVars))
    , m_maxOrder(std::max(1, maxOrder))
{
}

/** @brief 创建独立变量TPSA @param varIndex 变量索引 @return 变量TPSA */
TpsaEngine::Tpsa TpsaEngine::makeVariable(int varIndex)
{
    if (varIndex < 0 || varIndex >= m_numVars) return makeConstant(0.0);

    Tpsa t;
    t.order = m_maxOrder;
    t.numVars = m_numVars;

    MonoIndex zero(m_numVars, 0);
    t.terms[zero] = 0.0;

    MonoIndex mono(m_numVars, 0);
    mono[varIndex] = 1;
    t.terms[mono] = 1.0;

    ++m_stats.totalVariablesCreated;
    return t;
}

/** @brief 创建常数TPSA @param value 常数值 @return 常数TPSA */
TpsaEngine::Tpsa TpsaEngine::makeConstant(double value) const
{
    Tpsa t;
    t.order = m_maxOrder;
    t.numVars = m_numVars;
    MonoIndex zero(m_numVars, 0);
    t.terms[zero] = value;
    return t;
}

/** @brief 单项式阶数 @param mono 单项式 @return 阶数 */
int TpsaEngine::monoOrder(const MonoIndex& mono) const
{
    int o = 0;
    for (int e : mono) o += e;
    return o;
}

/** @brief 截断超阶项 @param tpsa TPSA */
void TpsaEngine::truncate(Tpsa& tpsa) const
{
    std::vector<MonoIndex> toRemove;
    for (const auto& [mono, coeff] : tpsa.terms) {
        if (monoOrder(mono) > m_maxOrder || std::abs(coeff) < 1e-15) {
            toRemove.push_back(mono);
        }
    }
    for (const auto& m : toRemove) {
        tpsa.terms.erase(m);
    }
}

/** @brief TPSA加法 @param a A @param b B @return a+b */
TpsaEngine::Tpsa TpsaEngine::add(const Tpsa& a, const Tpsa& b) const
{
    Tpsa result = a;
    result.order = m_maxOrder;
    for (const auto& [mono, coeff] : b.terms) {
        result.terms[mono] += coeff;
    }
    return result;
}

/** @brief TPSA减法 @param a A @param b B @return a-b */
TpsaEngine::Tpsa TpsaEngine::subtract(const Tpsa& a, const Tpsa& b) const
{
    Tpsa result = a;
    result.order = m_maxOrder;
    for (const auto& [mono, coeff] : b.terms) {
        result.terms[mono] -= coeff;
    }
    return result;
}

/** @brief TPSA乘法 @param a A @param b B @return a*b */
TpsaEngine::Tpsa TpsaEngine::multiply(const Tpsa& a, const Tpsa& b)
{
    QElapsedTimer timer;
    timer.start();

    Tpsa result;
    result.order = m_maxOrder;
    result.numVars = m_numVars;

    for (const auto& [ma, ca] : a.terms) {
        for (const auto& [mb, cb] : b.terms) {
            MonoIndex product(m_numVars);
            for (int i = 0; i < m_numVars; ++i) {
                product[i] = ma[i] + mb[i];
            }
            result.terms[product] += ca * cb;
        }
    }
    truncate(result);

    m_stats.totalTermsProcessed += static_cast<quint64>(result.termCount());
    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("multiply"));
    return result;
}

/** @brief TPSA除法 @param a 被除数 @param b 除数 @return a/b */
TpsaEngine::Tpsa TpsaEngine::divide(const Tpsa& a, const Tpsa& b)
{
    /* a/b = a * b^(-1) */
    Tpsa invB = pow(b, -1.0);
    Tpsa result = multiply(a, invB);
    emit operationCompleted(tr("divide"));
    return result;
}

/** @brief 泰勒展开 sin(d) @param d TPSA @return sin(d)近似 */
TpsaEngine::Tpsa TpsaEngine::taylorSin(const Tpsa& d)
{
    /* sin(d) = d - d^3/3! + d^5/5! - ... */
    Tpsa result = makeConstant(0.0);
    Tpsa dPow = makeConstant(1.0);
    double sign = 1.0;
    double fact = 1.0;

    for (int k = 1; k <= m_maxOrder; k += 2) {
        /* 累乘 d^(k) */
        while (monoOrder(dPow.terms.begin()->first) < k) {
            dPow = multiply(dPow, d);
        }
        double coeff = sign / fact;
        Tpsa term = multiply(makeConstant(coeff), dPow);
        result = add(result, term);
        sign = -sign;
        fact *= static_cast<double>(k + 1) * static_cast<double>(k + 2);
    }
    return result;
}

/** @brief 泰勒展开 cos(d) @param d TPSA @return cos(d)近似 */
TpsaEngine::Tpsa TpsaEngine::taylorCos(const Tpsa& d)
{
    /* cos(d) = 1 - d^2/2! + d^4/4! - ... */
    Tpsa result = makeConstant(1.0);
    Tpsa dPow = makeConstant(1.0);
    double sign = -1.0;
    double fact = 2.0;

    for (int k = 2; k <= m_maxOrder; k += 2) {
        dPow = multiply(dPow, multiply(d, d));
        if (k > 2) {
            dPow = multiply(dPow, d);
            dPow = multiply(dPow, d);
        }
        double coeff = sign / fact;
        Tpsa term = multiply(makeConstant(coeff), dPow);
        result = add(result, term);
        sign = -sign;
        fact *= static_cast<double>(k + 1) * static_cast<double>(k + 2);
    }
    return result;
}

/** @brief 泰勒展开 exp(d) @param d TPSA @return exp(d)近似 */
TpsaEngine::Tpsa TpsaEngine::taylorExp(const Tpsa& d)
{
    /* exp(d) = 1 + d + d^2/2! + d^3/3! + ... */
    Tpsa result = makeConstant(1.0);
    Tpsa dPow = d;
    double fact = 1.0;

    for (int k = 1; k <= m_maxOrder; ++k) {
        fact *= static_cast<double>(k);
        Tpsa term = multiply(makeConstant(1.0 / fact), dPow);
        result = add(result, term);
        if (k < m_maxOrder) {
            dPow = multiply(dPow, d);
        }
    }
    return result;
}

/** @brief TPSA正弦 @param a 操作数 @return sin(a) */
TpsaEngine::Tpsa TpsaEngine::sin(const Tpsa& a)
{
    QElapsedTimer timer;
    timer.start();

    /* sin(a) = sin(c)*cos(d) + cos(c)*sin(d), d = a - c, c = a.constant() */
    double c = a.constant();
    Tpsa d = subtract(a, makeConstant(c));

    Tpsa sinD = taylorSin(d);
    Tpsa cosD = taylorCos(d);
    Tpsa result = add(multiply(makeConstant(std::sin(c)), cosD),
                      multiply(makeConstant(std::cos(c)), sinD));

    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("sin"));
    return result;
}

/** @brief TPSA余弦 @param a 操作数 @return cos(a) */
TpsaEngine::Tpsa TpsaEngine::cos(const Tpsa& a)
{
    QElapsedTimer timer;
    timer.start();

    double c = a.constant();
    Tpsa d = subtract(a, makeConstant(c));

    Tpsa sinD = taylorSin(d);
    Tpsa cosD = taylorCos(d);
    Tpsa result = subtract(multiply(makeConstant(std::cos(c)), cosD),
                           multiply(makeConstant(std::sin(c)), sinD));

    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("cos"));
    return result;
}

/** @brief TPSA指数 @param a 操作数 @return exp(a) */
TpsaEngine::Tpsa TpsaEngine::exp(const Tpsa& a)
{
    QElapsedTimer timer;
    timer.start();

    /* exp(a) = exp(c) * exp(d), d = a - c */
    double c = a.constant();
    Tpsa d = subtract(a, makeConstant(c));
    Tpsa result = multiply(makeConstant(std::exp(c)), taylorExp(d));

    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("exp"));
    return result;
}

/** @brief TPSA对数 @param a 操作数 @return ln(a) */
TpsaEngine::Tpsa TpsaEngine::log(const Tpsa& a)
{
    QElapsedTimer timer;
    timer.start();

    /* ln(a) = ln(c) + ln(1 + d/c), d = a - c, c = a.constant() */
    double c = a.constant();
    Tpsa d = subtract(a, makeConstant(c));
    /* ln(1 + x) = x - x^2/2 + x^3/3 - ... where x = d/c */
    Tpsa x = multiply(makeConstant(1.0 / c), d);
    Tpsa xPow = x;
    Tpsa lnPart = makeConstant(0.0);

    for (int k = 1; k <= m_maxOrder; ++k) {
        Tpsa term = multiply(makeConstant((k % 2 == 0 ? -1.0 : 1.0) / k), xPow);
        lnPart = add(lnPart, term);
        if (k < m_maxOrder) {
            xPow = multiply(xPow, x);
        }
    }

    Tpsa result = add(makeConstant(std::log(c)), lnPart);

    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("log"));
    return result;
}

/** @brief TPSA幂 @param base 底数 @param exponent 指数 @return base^exponent */
TpsaEngine::Tpsa TpsaEngine::pow(const Tpsa& base, double exponent)
{
    QElapsedTimer timer;
    timer.start();

    /* base^e = exp(e * ln(base)) */
    Tpsa lnBase = log(base);
    Tpsa result = exp(multiply(makeConstant(exponent), lnBase));

    ++m_stats.totalOperations;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalOperations);

    emit operationCompleted(tr("pow"));
    return result;
}

/** @brief TPSA平方根 @param a 操作数 @return sqrt(a) */
TpsaEngine::Tpsa TpsaEngine::sqrt(const Tpsa& a)
{
    return pow(a, 0.5);
}

/** @brief 重置统计 */
void TpsaEngine::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
