/**
 * @file algo_5709.cpp
 */
#include "code5709/algo_5709.h"
QVector<double> algo_5709::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
