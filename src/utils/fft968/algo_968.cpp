/**
 * @file algo_968.cpp
 * @brief Algorithm module 968
 */
#include "fft968/algo_968.h"
QVector<double> algo_968::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
