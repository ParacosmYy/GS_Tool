/**
 * @file algo_2625.cpp
 * @brief Algorithm module 2625
 */
#include "matrix2625/algo_2625.h"
QVector<double> algo_2625::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
