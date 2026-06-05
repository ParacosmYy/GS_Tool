/**
 * @file algo_2412.cpp
 * @brief Algorithm module 2412
 */
#include "compress2412/algo_2412.h"
QVector<double> algo_2412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
