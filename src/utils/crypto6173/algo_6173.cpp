/**
 * @file algo_6173.cpp
 */
#include "crypto6173/algo_6173.h"
QVector<double> algo_6173::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
