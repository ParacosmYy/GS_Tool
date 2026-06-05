#pragma once
#include <QObject>
class EQMatch : public QObject { Q_OBJECT public: explicit EQMatch(QObject* p=nullptr) : QObject(p) {} };
