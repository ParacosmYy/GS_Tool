/**
 * @file PredictorCorrect2.cpp
 * @brief PredictorCorrect2 implementation
 */
#include "neural229/PredictorCorrect2.h"
#include <QElapsedTimer>
QVector<double> PredictorCorrect2::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

