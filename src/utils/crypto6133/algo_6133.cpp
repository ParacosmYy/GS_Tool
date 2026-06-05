/**
 * @file algo_6133.cpp
 */
#include "crypto6133/algo_6133.h"
QVector<double> algo_6133::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
