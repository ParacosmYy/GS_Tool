/**
 * @file algo_7247.cpp
 */
#include "dsp7247/algo_7247.h"
QVector<double> algo_7247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
