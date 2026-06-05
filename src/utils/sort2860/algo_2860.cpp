/**
 * @file algo_2860.cpp
 */
#include "sort2860/algo_2860.h"
QVector<double> algo_2860::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
