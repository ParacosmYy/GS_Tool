#pragma once
#include <QObject>
class Chromagram : public QObject { Q_OBJECT public: explicit Chromagram(QObject* p=nullptr) : QObject(p) {} };
