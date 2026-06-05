/**
 * @file algo_1048.cpp
 * @brief Algorithm module 1048
 */
#include "fft1048/algo_1048.h"
QVector<double> algo_1048::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
