/**
 * @file algo_7007.cpp
 */
#include "dsp7007/algo_7007.h"
QVector<double> algo_7007::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
