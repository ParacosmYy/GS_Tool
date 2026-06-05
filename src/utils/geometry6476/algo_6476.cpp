/**
 * @file algo_6476.cpp
 */
#include "geometry6476/algo_6476.h"
QVector<double> algo_6476::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
