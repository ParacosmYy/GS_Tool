/**
 * @file algo_2608.cpp
 * @brief Algorithm module 2608
 */
#include "fft2608/algo_2608.h"
QVector<double> algo_2608::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
