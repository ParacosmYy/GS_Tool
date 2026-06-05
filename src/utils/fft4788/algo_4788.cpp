/**
 * @file algo_4788.cpp
 */
#include "fft4788/algo_4788.h"
QVector<double> algo_4788::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
