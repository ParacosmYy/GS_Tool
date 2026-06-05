/**
 * @file interp__701.cpp
 * @brief interp__701 implementation
 */
#include "interp701/interp__701.h"
QVector<double> interp__701::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

