/**
 * @file sort__320.cpp
 * @brief sort__320 implementation
 */
#include "sort320/sort__320.h"
QVector<double> sort__320::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

