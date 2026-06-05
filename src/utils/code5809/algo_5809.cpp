/**
 * @file algo_5809.cpp
 */
#include "code5809/algo_5809.h"
QVector<double> algo_5809::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
