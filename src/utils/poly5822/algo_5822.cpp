/**
 * @file algo_5822.cpp
 */
#include "poly5822/algo_5822.h"
QVector<double> algo_5822::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
