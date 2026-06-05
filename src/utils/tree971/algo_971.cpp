/**
 * @file algo_971.cpp
 * @brief Algorithm module 971
 */
#include "tree971/algo_971.h"
QVector<double> algo_971::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
