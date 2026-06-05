/**
 * @file algo_3467.cpp
 */
#include "dsp3467/algo_3467.h"
QVector<double> algo_3467::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
