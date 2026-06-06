/**
 * @file algo_7447.cpp
 */
#include "dsp7447/algo_7447.h"
QVector<double> algo_7447::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
