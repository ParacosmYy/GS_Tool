#pragma once
#include <QObject>
#include <QVector>
class m18651 : public QObject { Q_OBJECT public: struct Stats { quint64 c=0; quint64 i=0; }; explicit m18651(QObject *p=nullptr):QObject(p){} QVector<double> run(const QVector<double>&in); Stats s() const { return m_s; } private: Stats m_s; };
