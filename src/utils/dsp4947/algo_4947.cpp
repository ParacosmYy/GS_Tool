/**
 * @file algo_4947.cpp
 */
#include "dsp4947/algo_4947.h"
QVector<double> algo_4947::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
