/**
 * @file algo_4809.cpp
 */
#include "code4809/algo_4809.h"
QVector<double> algo_4809::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
