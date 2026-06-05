/**
 * @file algo_4332.cpp
 */
#include "compress4332/algo_4332.h"
QVector<double> algo_4332::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
