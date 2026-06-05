/**
 * @file algo_6375.cpp
 */
#include "optim6375/algo_6375.h"
QVector<double> algo_6375::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
