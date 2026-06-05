#pragma once
#include <QObject>
class WavetableOsc : public QObject { Q_OBJECT public: explicit WavetableOsc(QObject* p=nullptr) : QObject(p) {} };
