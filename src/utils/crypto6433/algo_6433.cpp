/**
 * @file algo_6433.cpp
 */
#include "crypto6433/algo_6433.h"
QVector<double> algo_6433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
