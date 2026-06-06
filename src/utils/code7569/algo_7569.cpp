/**
 * @file algo_7569.cpp
 */
#include "code7569/algo_7569.h"
QVector<double> algo_7569::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
