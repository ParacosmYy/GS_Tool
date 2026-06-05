/**
 * @file algo_6011.cpp
 */
#include "tree6011/algo_6011.h"
QVector<double> algo_6011::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
