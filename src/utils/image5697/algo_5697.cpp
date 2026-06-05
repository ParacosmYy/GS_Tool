/**
 * @file algo_5697.cpp
 */
#include "image5697/algo_5697.h"
QVector<double> algo_5697::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
