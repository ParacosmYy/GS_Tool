#pragma once
#include <QObject>
class CrcAccelerator : public QObject { Q_OBJECT public: explicit CrcAccelerator(QObject* p=nullptr) : QObject(p) {} };
