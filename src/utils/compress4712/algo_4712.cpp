/**
 * @file algo_4712.cpp
 */
#include "compress4712/algo_4712.h"
QVector<double> algo_4712::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
