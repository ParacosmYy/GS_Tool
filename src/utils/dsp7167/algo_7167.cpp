/**
 * @file algo_7167.cpp
 */
#include "dsp7167/algo_7167.h"
QVector<double> algo_7167::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
