/**
 * @file algo_828.cpp
 * @brief Algorithm module 828
 */
#include "fft828/algo_828.h"
QVector<double> algo_828::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
