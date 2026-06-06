/**
 * @file algo_7187.cpp
 */
#include "dsp7187/algo_7187.h"
QVector<double> algo_7187::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
