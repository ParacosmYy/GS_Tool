/**
 * @file algo_4603.cpp
 */
#include "string4603/algo_4603.h"
QVector<double> algo_4603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
