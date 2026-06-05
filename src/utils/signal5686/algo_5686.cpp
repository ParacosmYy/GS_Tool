/**
 * @file algo_5686.cpp
 */
#include "signal5686/algo_5686.h"
QVector<double> algo_5686::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
