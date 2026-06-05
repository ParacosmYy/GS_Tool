/**
 * @file algo_3187.cpp
 */
#include "dsp3187/algo_3187.h"
QVector<double> algo_3187::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
