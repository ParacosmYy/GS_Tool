/**
 * @file algo_6347.cpp
 */
#include "dsp6347/algo_6347.h"
QVector<double> algo_6347::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
