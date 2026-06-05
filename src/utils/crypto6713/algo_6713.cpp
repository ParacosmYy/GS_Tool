/**
 * @file algo_6713.cpp
 */
#include "crypto6713/algo_6713.h"
QVector<double> algo_6713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
