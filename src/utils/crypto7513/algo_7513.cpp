/**
 * @file algo_7513.cpp
 */
#include "crypto7513/algo_7513.h"
QVector<double> algo_7513::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
