/**
 * @file algo_6429.cpp
 */
#include "code6429/algo_6429.h"
QVector<double> algo_6429::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
