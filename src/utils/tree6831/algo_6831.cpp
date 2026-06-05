/**
 * @file algo_6831.cpp
 */
#include "tree6831/algo_6831.h"
QVector<double> algo_6831::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
