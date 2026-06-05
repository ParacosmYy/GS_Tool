/**
 * @file algo_2432.cpp
 * @brief Algorithm module 2432
 */
#include "compress2432/algo_2432.h"
QVector<double> algo_2432::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
