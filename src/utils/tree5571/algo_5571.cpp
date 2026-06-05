/**
 * @file algo_5571.cpp
 */
#include "tree5571/algo_5571.h"
QVector<double> algo_5571::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
