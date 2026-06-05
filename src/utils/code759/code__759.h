/**
 * @file code__759.h
 * @brief code module code__759
 */
#pragma once
#include <QObject>
#include <QVector>
class code__759 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit code__759(QObject *p=nullptr) : QObject(p) {}
    ~code__759() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

