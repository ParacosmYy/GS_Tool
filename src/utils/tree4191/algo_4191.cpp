/**
 * @file algo_4191.cpp
 */
#include "tree4191/algo_4191.h"
QVector<double> algo_4191::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
