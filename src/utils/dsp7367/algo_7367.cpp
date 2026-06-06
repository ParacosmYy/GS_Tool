/**
 * @file algo_7367.cpp
 */
#include "dsp7367/algo_7367.h"
QVector<double> algo_7367::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
