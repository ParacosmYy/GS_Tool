/**
 * @file interp__721.cpp
 * @brief interp__721 implementation
 */
#include "interp721/interp__721.h"
QVector<double> interp__721::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

