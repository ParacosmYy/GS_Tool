/**
 * @file algo_6748.cpp
 */
#include "fft6748/algo_6748.h"
QVector<double> algo_6748::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
