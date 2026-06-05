/**
 * @file algo_1789.cpp
 * @brief Algorithm module 1789
 */
#include "code1789/algo_1789.h"
QVector<double> algo_1789::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
