/**
 * @file algo_2388.cpp
 * @brief Algorithm module 2388
 */
#include "fft2388/algo_2388.h"
QVector<double> algo_2388::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
