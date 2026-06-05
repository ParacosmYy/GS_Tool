/**
 * @file algo_1821.cpp
 * @brief Algorithm module 1821
 */
#include "interp1821/algo_1821.h"
QVector<double> algo_1821::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
