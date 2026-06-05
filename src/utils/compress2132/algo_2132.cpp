/**
 * @file algo_2132.cpp
 * @brief Algorithm module 2132
 */
#include "compress2132/algo_2132.h"
QVector<double> algo_2132::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
