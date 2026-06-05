#pragma once
#include <QObject>
class AudioFingerprint : public QObject { Q_OBJECT public: explicit AudioFingerprint(QObject* p=nullptr) : QObject(p) {} };
