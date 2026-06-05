#pragma once
#include <QObject>
class Limiter : public QObject { Q_OBJECT public: explicit Limiter(QObject* p=nullptr) : QObject(p) {} };
