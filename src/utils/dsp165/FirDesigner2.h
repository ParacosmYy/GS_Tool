/**
 * @file FirDesigner2.h
 * @brief FIR filter design using windowing method
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief FIR filter design using windowing method
 */
class FirDesigner2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit FirDesigner2(QObject *p = nullptr) : QObject(p) {}
    ~FirDesigner2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

