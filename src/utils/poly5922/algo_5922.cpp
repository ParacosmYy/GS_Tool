/**
 * @file algo_5922.cpp
 */
#include "poly5922/algo_5922.h"
QVector<double> algo_5922::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
