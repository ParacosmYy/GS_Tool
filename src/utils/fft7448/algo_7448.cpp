/**
 * @file algo_7448.cpp
 */
#include "fft7448/algo_7448.h"
QVector<double> algo_7448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
