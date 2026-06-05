/**
 * @file algo_3194.cpp
 */
#include "numeric3194/algo_3194.h"
QVector<double> algo_3194::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
