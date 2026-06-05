/**
 * @file FuzzyLogicEngine.cpp
 * @brief 模糊逻辑引擎实现
 */

#include "utils/fuzzy2/FuzzyLogicEngine.h"
#include <QElapsedTimer>
#include <QtMath>

FuzzyLogicEngine::FuzzyLogicEngine(QObject* parent)
    : QObject(parent), m_outputMin(0.0), m_outputMax(100.0),
      m_outputResolution(100), m_timeSum(0.0) {}

void FuzzyLogicEngine::addMembershipFunction(const QString& varName, const QString& termName,
                                              MfType type, const QVector<double>& params)
{
    m_inputMfs[varName][termName] = {type, params};
}

void FuzzyLogicEngine::setOutputRange(double minVal, double maxVal, int resolution)
{
    m_outputMin = minVal;
    m_outputMax = maxVal;
    m_outputResolution = qMax(10, resolution);
}

void FuzzyLogicEngine::addRule(const Rule& rule) { m_rules.append(rule); }

double FuzzyLogicEngine::evaluate(const QMap<QString, double>& inputs)
{
    QElapsedTimer timer;
    timer.start();

    /* 1. 模糊化: 计算每个输入对每个隶属函数的隶属度 */
    QMap<QString, QMap<QString, double>> fuzzified;
    for (auto it = m_inputMfs.begin(); it != m_inputMfs.end(); ++it) {
        const QString& varName = it.key();
        double inputVal = inputs.value(varName, 0.0);
        for (auto mfIt = it.value().begin(); mfIt != it.value().end(); ++mfIt) {
            fuzzified[varName][mfIt.key()] = membership(inputVal, mfIt.value().type, mfIt.value().params);
        }
    }

    /* 2. 规则推理: 计算每条规则的激活强度(取最小AND) */
    int rulesFired = 0;
    QMap<QString, double> outputActivations;

    for (const Rule& rule : m_rules) {
        double strength = 1.0;
        for (auto condIt = rule.conditions.begin(); condIt != rule.conditions.end(); ++condIt) {
            const QString& var = condIt.key();
            const QString& term = condIt.value();
            double mu = fuzzified.value(var).value(term, 0.0);
            strength = qMin(strength, mu);
        }

        if (strength > 0.0) {
            double current = outputActivations.value(rule.outputTerm, 0.0);
            outputActivations[rule.outputTerm] = qMax(current, strength);
            ++rulesFired;
        }
    }

    /* 3. 去模糊化: 质心法 */
    double step = (m_outputMax - m_outputMin) / m_outputResolution;
    double numerator = 0.0, denominator = 0.0;

    for (int i = 0; i <= m_outputResolution; ++i) {
        double x = m_outputMin + i * step;
        double aggMu = 0.0;
        for (auto it = outputActivations.begin(); it != outputActivations.end(); ++it) {
            aggMu = qMax(aggMu, it.value());
        }

        /* 简化: 使用最大激活度作为聚合隶属度 */
        double maxActivation = 0.0;
        for (auto it = outputActivations.begin(); it != outputActivations.end(); ++it)
            maxActivation = qMax(maxActivation, it.value());

        numerator += x * maxActivation;
        denominator += maxActivation;
    }

    double result = (denominator > 1e-15) ? numerator / denominator : (m_outputMin + m_outputMax) / 2.0;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalEvaluations;
    m_stats.totalRulesFired += rulesFired;
    m_stats.avgRulesFired = static_cast<double>(m_stats.totalRulesFired) / m_stats.totalEvaluations;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalEvaluations;

    emit evaluationComplete(result, rulesFired);
    return result;
}

double FuzzyLogicEngine::membership(double x, MfType type, const QVector<double>& p) const
{
    switch (type) {
    case MfType::Triangle:
        if (p.size() < 3) return 0.0;
        if (x <= p[0] || x >= p[2]) return 0.0;
        if (x <= p[1]) return (x - p[0]) / (p[1] - p[0] + 1e-15);
        return (p[2] - x) / (p[2] - p[1] + 1e-15);

    case MfType::Trapezoid:
        if (p.size() < 4) return 0.0;
        if (x < p[0] || x > p[3]) return 0.0;
        if (x >= p[1] && x <= p[2]) return 1.0;
        if (x < p[1]) return (x - p[0]) / (p[1] - p[0] + 1e-15);
        return (p[3] - x) / (p[3] - p[2] + 1e-15);

    case MfType::Gaussian:
        if (p.size() < 2) return 0.0;
        return qExp(-0.5 * qPow((x - p[0]) / (p[1] + 1e-15), 2));
    }
    return 0.0;
}

void FuzzyLogicEngine::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
