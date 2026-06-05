/**
 * @file algo_1231.cpp
 * @brief Algorithm module 1231
 */
#include "tree1231/algo_1231.h"
QVector<double> algo_1231::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
