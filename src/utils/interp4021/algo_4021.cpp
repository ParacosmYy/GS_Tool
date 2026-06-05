/**
 * @file algo_4021.cpp
 */
#include "interp4021/algo_4021.h"
QVector<double> algo_4021::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
