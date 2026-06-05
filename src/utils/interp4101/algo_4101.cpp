/**
 * @file algo_4101.cpp
 */
#include "interp4101/algo_4101.h"
QVector<double> algo_4101::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
