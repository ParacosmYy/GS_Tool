/**
 * @file algo_4023.cpp
 */
#include "string4023/algo_4023.h"
QVector<double> algo_4023::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
