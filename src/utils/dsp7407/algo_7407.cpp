/**
 * @file algo_7407.cpp
 */
#include "dsp7407/algo_7407.h"
QVector<double> algo_7407::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
