#pragma once
#include <QObject>
class VoiceActivityDetect : public QObject { Q_OBJECT public: explicit VoiceActivityDetect(QObject* p=nullptr) : QObject(p) {} };
