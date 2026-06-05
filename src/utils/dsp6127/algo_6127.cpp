/**
 * @file algo_6127.cpp
 */
#include "dsp6127/algo_6127.h"
QVector<double> algo_6127::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
