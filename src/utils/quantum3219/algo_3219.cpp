/**
 * @file algo_3219.cpp
 */
#include "quantum3219/algo_3219.h"
QVector<double> algo_3219::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
