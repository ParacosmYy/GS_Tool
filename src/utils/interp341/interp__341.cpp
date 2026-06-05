/**
 * @file interp__341.cpp
 * @brief interp__341 implementation
 */
#include "interp341/interp__341.h"
QVector<double> interp__341::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

