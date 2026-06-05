/**
 * @file image__417.h
 * @brief image module image__417
 */
#pragma once
#include <QObject>
#include <QVector>
class image__417 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit image__417(QObject *p=nullptr) : QObject(p) {}
    ~image__417() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

