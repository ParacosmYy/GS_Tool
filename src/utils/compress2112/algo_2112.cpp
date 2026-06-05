/**
 * @file algo_2112.cpp
 * @brief Algorithm module 2112
 */
#include "compress2112/algo_2112.h"
QVector<double> algo_2112::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
