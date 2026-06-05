/**
 * @file algo_5763.cpp
 */
#include "string5763/algo_5763.h"
QVector<double> algo_5763::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
