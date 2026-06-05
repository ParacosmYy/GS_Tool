/**
 * @file algo_1388.cpp
 * @brief Algorithm module 1388
 */
#include "fft1388/algo_1388.h"
QVector<double> algo_1388::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
