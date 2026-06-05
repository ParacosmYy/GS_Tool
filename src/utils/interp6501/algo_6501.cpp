/**
 * @file algo_6501.cpp
 */
#include "interp6501/algo_6501.h"
QVector<double> algo_6501::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
