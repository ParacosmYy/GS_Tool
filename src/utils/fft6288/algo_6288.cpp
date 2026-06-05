/**
 * @file algo_6288.cpp
 */
#include "fft6288/algo_6288.h"
QVector<double> algo_6288::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
