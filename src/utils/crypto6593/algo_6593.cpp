/**
 * @file algo_6593.cpp
 */
#include "crypto6593/algo_6593.h"
QVector<double> algo_6593::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
