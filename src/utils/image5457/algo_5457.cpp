/**
 * @file algo_5457.cpp
 */
#include "image5457/algo_5457.h"
QVector<double> algo_5457::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
