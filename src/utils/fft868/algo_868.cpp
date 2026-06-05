/**
 * @file algo_868.cpp
 * @brief Algorithm module 868
 */
#include "fft868/algo_868.h"
QVector<double> algo_868::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
