/**
 * @file algo_6728.cpp
 */
#include "fft6728/algo_6728.h"
QVector<double> algo_6728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
