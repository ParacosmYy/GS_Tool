/**
 * @file algo_3397.cpp
 */
#include "image3397/algo_3397.h"
QVector<double> algo_3397::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
