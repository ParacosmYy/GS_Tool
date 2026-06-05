/**
 * @file algo_1648.cpp
 * @brief Algorithm module 1648
 */
#include "fft1648/algo_1648.h"
QVector<double> algo_1648::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
