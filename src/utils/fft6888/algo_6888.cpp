/**
 * @file algo_6888.cpp
 */
#include "fft6888/algo_6888.h"
QVector<double> algo_6888::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
