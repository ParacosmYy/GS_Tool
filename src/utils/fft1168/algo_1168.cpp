/**
 * @file algo_1168.cpp
 * @brief Algorithm module 1168
 */
#include "fft1168/algo_1168.h"
QVector<double> algo_1168::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
