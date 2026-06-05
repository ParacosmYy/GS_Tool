/**
 * @file algo_2472.cpp
 * @brief Algorithm module 2472
 */
#include "compress2472/algo_2472.h"
QVector<double> algo_2472::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
