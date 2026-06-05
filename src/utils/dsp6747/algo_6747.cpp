/**
 * @file algo_6747.cpp
 */
#include "dsp6747/algo_6747.h"
QVector<double> algo_6747::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
