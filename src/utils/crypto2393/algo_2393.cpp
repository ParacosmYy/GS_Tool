/**
 * @file algo_2393.cpp
 * @brief Algorithm module 2393
 */
#include "crypto2393/algo_2393.h"
QVector<double> algo_2393::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
