/**
 * @file algo_1741.cpp
 * @brief Algorithm module 1741
 */
#include "interp1741/algo_1741.h"
QVector<double> algo_1741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
