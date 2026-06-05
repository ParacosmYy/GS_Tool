/**
 * @file algo_908.cpp
 * @brief Algorithm module 908
 */
#include "fft908/algo_908.h"
QVector<double> algo_908::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
