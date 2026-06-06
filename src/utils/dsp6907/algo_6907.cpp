/**
 * @file algo_6907.cpp
 */
#include "dsp6907/algo_6907.h"
QVector<double> algo_6907::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
