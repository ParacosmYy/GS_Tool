/**
 * @file crypto__483.h
 * @brief crypto module crypto__483
 */
#pragma once
#include <QObject>
#include <QVector>
class crypto__483 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit crypto__483(QObject *p=nullptr) : QObject(p) {}
    ~crypto__483() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

