/**
 * @file algo_813.cpp
 * @brief Algorithm module 813
 */
#include "crypto813/algo_813.h"
QVector<double> algo_813::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
