/**
 * @file algo_2612.cpp
 * @brief Algorithm module 2612
 */
#include "compress2612/algo_2612.h"
QVector<double> algo_2612::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
