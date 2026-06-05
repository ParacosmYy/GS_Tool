/**
 * @file algo_921.cpp
 * @brief Algorithm module 921
 */
#include "interp921/algo_921.h"
QVector<double> algo_921::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
