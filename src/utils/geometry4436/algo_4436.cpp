/**
 * @file algo_4436.cpp
 */
#include "geometry4436/algo_4436.h"
QVector<double> algo_4436::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
