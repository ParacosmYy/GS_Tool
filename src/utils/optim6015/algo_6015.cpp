/**
 * @file algo_6015.cpp
 */
#include "optim6015/algo_6015.h"
QVector<double> algo_6015::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
