/**
 * @file algo_6448.cpp
 */
#include "fft6448/algo_6448.h"
QVector<double> algo_6448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
