/**
 * @file algo_1963.cpp
 * @brief Algorithm module 1963
 */
#include "string1963/algo_1963.h"
QVector<double> algo_1963::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
