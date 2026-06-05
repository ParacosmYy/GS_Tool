/**
 * @file algo_6072.cpp
 */
#include "compress6072/algo_6072.h"
QVector<double> algo_6072::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
