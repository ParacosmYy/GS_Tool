/**
 * @file CholeskyDecomp2.h
 * @brief interp algorithm module - CholeskyDecomp2
 */
#pragma once
#include <QObject>
#include <QVector>
class CholeskyDecomp2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit CholeskyDecomp2(QObject *p = nullptr) : QObject(p) {}
    ~CholeskyDecomp2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

