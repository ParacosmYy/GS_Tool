/**
 * @file algo_6989.cpp
 */
#include "code6989/algo_6989.h"
QVector<double> algo_6989::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
