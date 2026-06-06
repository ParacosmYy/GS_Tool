/**
 * @file algo_7207.cpp
 */
#include "dsp7207/algo_7207.h"
QVector<double> algo_7207::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
