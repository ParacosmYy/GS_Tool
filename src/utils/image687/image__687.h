/**
 * @file image__687.h
 * @brief image module image__687
 */
#pragma once
#include <QObject>
#include <QVector>
class image__687 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit image__687(QObject *p=nullptr) : QObject(p) {}
    ~image__687() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

