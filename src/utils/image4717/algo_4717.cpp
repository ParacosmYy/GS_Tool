/**
 * @file algo_4717.cpp
 */
#include "image4717/algo_4717.h"
QVector<double> algo_4717::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
