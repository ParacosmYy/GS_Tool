/**
 * @file algo_4940.cpp
 */
#include "sort4940/algo_4940.h"
QVector<double> algo_4940::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
