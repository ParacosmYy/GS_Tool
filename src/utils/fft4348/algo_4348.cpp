/**
 * @file algo_4348.cpp
 */
#include "fft4348/algo_4348.h"
QVector<double> algo_4348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
