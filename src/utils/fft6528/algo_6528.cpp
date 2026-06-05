/**
 * @file algo_6528.cpp
 */
#include "fft6528/algo_6528.h"
QVector<double> algo_6528::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
