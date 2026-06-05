/**
 * @file algo_4441.cpp
 */
#include "interp4441/algo_4441.h"
QVector<double> algo_4441::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
