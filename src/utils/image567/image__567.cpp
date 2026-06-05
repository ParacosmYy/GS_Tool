/**
 * @file image__567.cpp
 * @brief image__567 implementation
 */
#include "image567/image__567.h"
QVector<double> image__567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

