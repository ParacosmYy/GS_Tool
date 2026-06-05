/**
 * @file algo_3557.cpp
 */
#include "image3557/algo_3557.h"
QVector<double> algo_3557::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
