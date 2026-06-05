/**
 * @file algo_6773.cpp
 */
#include "crypto6773/algo_6773.h"
QVector<double> algo_6773::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
