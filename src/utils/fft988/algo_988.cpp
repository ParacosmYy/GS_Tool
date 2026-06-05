/**
 * @file algo_988.cpp
 * @brief Algorithm module 988
 */
#include "fft988/algo_988.h"
QVector<double> algo_988::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
