/**
 * @file algo_4860.cpp
 */
#include "sort4860/algo_4860.h"
QVector<double> algo_4860::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
