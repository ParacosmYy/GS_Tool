/**
 * @file algo_1468.cpp
 * @brief Algorithm module 1468
 */
#include "fft1468/algo_1468.h"
QVector<double> algo_1468::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
