/**
 * @file optim__765.h
 * @brief optim module optim__765
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__765 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__765(QObject *p=nullptr) : QObject(p) {}
    ~optim__765() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

