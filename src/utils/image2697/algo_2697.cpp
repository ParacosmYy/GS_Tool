/**
 * @file algo_2697.cpp
 * @brief Algorithm module 2697
 */
#include "image2697/algo_2697.h"
QVector<double> algo_2697::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
