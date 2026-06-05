/**
 * @file algo_3841.cpp
 */
#include "interp3841/algo_3841.h"
QVector<double> algo_3841::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
