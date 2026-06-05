/**
 * @file algo_1553.cpp
 * @brief Algorithm module 1553
 */
#include "crypto1553/algo_1553.h"
QVector<double> algo_1553::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
