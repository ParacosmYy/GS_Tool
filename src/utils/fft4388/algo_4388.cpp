/**
 * @file algo_4388.cpp
 */
#include "fft4388/algo_4388.h"
QVector<double> algo_4388::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
