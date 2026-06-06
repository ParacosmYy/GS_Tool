/**
 * @file algo_7154.cpp
 */
#include "numeric7154/algo_7154.h"
QVector<double> algo_7154::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
