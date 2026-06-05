/**
 * @file algo_5440.cpp
 */
#include "sort5440/algo_5440.h"
QVector<double> algo_5440::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
