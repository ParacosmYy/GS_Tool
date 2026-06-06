/**
 * @file algo_6908.cpp
 */
#include "fft6908/algo_6908.h"
QVector<double> algo_6908::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
