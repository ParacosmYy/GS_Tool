/**
 * @file algo_7712.cpp
 */
#include "compress7712/algo_7712.h"
QVector<double> algo_7712::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
