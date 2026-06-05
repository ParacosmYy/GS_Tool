/**
 * @file algo_5029.cpp
 */
#include "code5029/algo_5029.h"
QVector<double> algo_5029::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
