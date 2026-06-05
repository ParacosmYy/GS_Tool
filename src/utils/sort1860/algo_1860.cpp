/**
 * @file algo_1860.cpp
 * @brief Algorithm module 1860
 */
#include "sort1860/algo_1860.h"
QVector<double> algo_1860::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
