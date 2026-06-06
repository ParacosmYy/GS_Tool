/**
 * @file algo_7189.cpp
 */
#include "code7189/algo_7189.h"
QVector<double> algo_7189::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
