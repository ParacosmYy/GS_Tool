/**
 * @file algo_1328.cpp
 * @brief Algorithm module 1328
 */
#include "fft1328/algo_1328.h"
QVector<double> algo_1328::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
