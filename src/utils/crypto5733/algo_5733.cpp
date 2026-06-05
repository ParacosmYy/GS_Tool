/**
 * @file algo_5733.cpp
 */
#include "crypto5733/algo_5733.h"
QVector<double> algo_5733::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
