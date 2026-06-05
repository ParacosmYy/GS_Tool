/**
 * @file algo_3388.cpp
 */
#include "fft3388/algo_3388.h"
QVector<double> algo_3388::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
