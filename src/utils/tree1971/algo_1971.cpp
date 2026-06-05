/**
 * @file algo_1971.cpp
 * @brief Algorithm module 1971
 */
#include "tree1971/algo_1971.h"
QVector<double> algo_1971::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
