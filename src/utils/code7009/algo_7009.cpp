/**
 * @file algo_7009.cpp
 */
#include "code7009/algo_7009.h"
QVector<double> algo_7009::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
