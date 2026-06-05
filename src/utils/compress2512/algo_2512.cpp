/**
 * @file algo_2512.cpp
 * @brief Algorithm module 2512
 */
#include "compress2512/algo_2512.h"
QVector<double> algo_2512::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
