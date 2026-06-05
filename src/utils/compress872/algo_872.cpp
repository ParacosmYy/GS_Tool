/**
 * @file algo_872.cpp
 * @brief Algorithm module 872
 */
#include "compress872/algo_872.h"
QVector<double> algo_872::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
