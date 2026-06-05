/**
 * @file algo_860.cpp
 * @brief Algorithm module 860
 */
#include "sort860/algo_860.h"
QVector<double> algo_860::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
