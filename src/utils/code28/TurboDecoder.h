#pragma once
#include <QObject>
class TurboDecoder : public QObject { Q_OBJECT public: explicit TurboDecoder(QObject* p=nullptr) : QObject(p) {} };
