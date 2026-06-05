/**
 * @file algo_823.cpp
 * @brief Algorithm module 823
 */
#include "string823/algo_823.h"
QVector<double> algo_823::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
