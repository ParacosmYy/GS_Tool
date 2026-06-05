/**
 * @file algo_1090.cpp
 * @brief Algorithm module 1090
 */
#include "cluster1090/algo_1090.h"
QVector<double> algo_1090::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
