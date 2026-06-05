/**
 * @file algo_4015.cpp
 */
#include "optim4015/algo_4015.h"
QVector<double> algo_4015::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
