/**
 * @file algo_4261.cpp
 */
#include "interp4261/algo_4261.h"
QVector<double> algo_4261::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
