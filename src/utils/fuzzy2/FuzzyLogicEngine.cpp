/**
 * @file FuzzyLogicEngine.cpp
 * @brief 模糊逻辑引擎实现 — Mamdani推理 + 重心法去模糊化
 */

#include "FuzzyLogicEngine.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

FuzzyLogicEngine::FuzzyLogicEngine(QObject* parent)
    : QObject(parent)
{
}

FuzzyLogicEngine::~FuzzyLogicEngine() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void FuzzyLogicEngine::addMembershipFunction(const QString& variable,
                                             const QString& term,
                                             const TrapezoidMF& mf)
{
    m_membershipFunctions[variable][term] = mf;

    if (!m_inputVariables.contains(variable)) {
        m_inputVariables.append(variable);
    }
}

void FuzzyLogicEngine::addTriangleMF(const QString& variable,
                                     const QString& term,
                                     double a, double b, double c)
{
    TrapezoidMF mf{a, b, b, c};
    addMembershipFunction(variable, term, mf);
}

void FuzzyLogicEngine::addRule(const QStringList& inputTerms,
                               const QString& outputVar,
                               const QString& outputTerm)
{
    FuzzyRule rule;
    rule.inputTerms = inputTerms;
    rule.outputVariable = outputVar;
    rule.outputTerm = outputTerm;
    m_rules.append(rule);
}

void FuzzyLogicEngine::setDefuzzResolution(int resolution)
{
    m_defuzzResolution = qMax(10, resolution);
}

// ═══════════════════════════════════════════════════════════
// 推理
// ═══════════════════════════════════════════════════════════

double FuzzyLogicEngine::evaluate(const QMap<QString, double>& inputs)
{
    QElapsedTimer timer;
    timer.start();

    // 1. 计算每条规则的触发强度(取输入隶属度的最小值)
    QMap<QString, double> clippedHeights;
    int rulesFired = 0;

    for (const auto& rule : m_rules) {
        double strength = 1.0;

        for (int i = 0; i < rule.inputTerms.size() &&
                        i < m_inputVariables.size(); ++i) {
            const QString& varName = m_inputVariables[i];
            const QString& termName = rule.inputTerms[i];

            if (!inputs.contains(varName)) {
                strength = 0.0;
                break;
            }

            const double deg = membershipDegree(varName, termName,
                                                inputs.value(varName));
            strength = qMin(strength, deg);
        }

        if (strength > 0.0) {
            rulesFired++;
            const QString& key = rule.outputTerm;
            if (clippedHeights.contains(key)) {
                clippedHeights[key] = qMax(clippedHeights[key], strength);
            } else {
                clippedHeights[key] = strength;
            }
        }
    }

    // 2. 去模糊化
    double result = centroidDefuzzify(clippedHeights);

    // 3. 更新统计
    m_stats.totalEvaluations += 1;
    m_stats.totalRulesFired += static_cast<quint64>(rulesFired);
    updateAvgTime(timer.nsecsElapsed() / 1000);

    emit evaluated(result, rulesFired);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 辅助
// ═══════════════════════════════════════════════════════════

double FuzzyLogicEngine::membershipDegree(const QString& variable,
                                          const QString& term,
                                          double x) const
{
    if (!m_membershipFunctions.contains(variable)) {
        return 0.0;
    }
    const auto& terms = m_membershipFunctions[variable];
    if (!terms.contains(term)) {
        return 0.0;
    }
    return trapezoidValue(terms[term], x);
}

void FuzzyLogicEngine::clear()
{
    m_membershipFunctions.clear();
    m_inputVariables.clear();
    m_rules.clear();
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

FuzzyLogicEngine::Stats FuzzyLogicEngine::stats() const
{
    return m_stats;
}

void FuzzyLogicEngine::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

double FuzzyLogicEngine::trapezoidValue(const TrapezoidMF& mf,
                                        double x) const
{
    if (x <= mf.a || x >= mf.d) {
        return 0.0;
    }
    if (x >= mf.b && x <= mf.c) {
        return 1.0;
    }
    if (x < mf.b) {
        return (mf.b - mf.a) > 0.0
            ? (x - mf.a) / (mf.b - mf.a) : 1.0;
    }
    return (mf.d - mf.c) > 0.0
        ? (mf.d - x) / (mf.d - mf.c) : 1.0;
}

double FuzzyLogicEngine::centroidDefuzzify(
    const QMap<QString, double>& clippedHeights) const
{
    if (clippedHeights.isEmpty()) {
        return 0.0;
    }

    // 确定输出范围
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();

    for (const auto& termEntry : m_membershipFunctions) {
        for (auto it = termEntry.constBegin();
             it != termEntry.constEnd(); ++it) {
            xMin = qMin(xMin, it.value().a);
            xMax = qMax(xMax, it.value().d);
        }
    }

    if (xMin >= xMax) {
        return 0.0;
    }

    // 重心法数值积分
    const double step = (xMax - xMin) /
                        static_cast<double>(m_defuzzResolution);
    double numerator = 0.0;
    double denominator = 0.0;

    for (int i = 0; i <= m_defuzzResolution; ++i) {
        const double x = xMin + static_cast<double>(i) * step;

        double aggValue = 0.0;
        for (auto it = clippedHeights.constBegin();
             it != clippedHeights.constEnd(); ++it) {
            for (const auto& varTerms : m_membershipFunctions) {
                if (varTerms.contains(it.key())) {
                    const double raw = trapezoidValue(
                        varTerms[it.key()], x);
                    const double clipped = qMin(raw, it.value());
                    aggValue = qMax(aggValue, clipped);
                }
            }
        }

        numerator += x * aggValue * step;
        denominator += aggValue * step;
    }

    return (denominator > 0.0) ? (numerator / denominator) : 0.0;
}

void FuzzyLogicEngine::updateAvgTime(qint64 elapsedUs)
{
    const auto n = m_stats.totalEvaluations;
    if (n == 1) {
        m_stats.avgProcessingTime = static_cast<double>(elapsedUs);
    } else {
        m_stats.avgProcessingTime =
            m_stats.avgProcessingTime *
                static_cast<double>(n - 1) / static_cast<double>(n) +
            static_cast<double>(elapsedUs) / static_cast<double>(n);
    }
}
