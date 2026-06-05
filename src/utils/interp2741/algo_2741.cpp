/**
 * @file algo_2741.cpp
 * @brief Algorithm module 2741
 */
#include "interp2741/algo_2741.h"
QVector<double> algo_2741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
