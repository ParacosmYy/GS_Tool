/**
 * @file geometry__686.h
 * @brief geometry module geometry__686
 */
#pragma once
#include <QObject>
#include <QVector>
class geometry__686 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit geometry__686(QObject *p=nullptr) : QObject(p) {}
    ~geometry__686() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

