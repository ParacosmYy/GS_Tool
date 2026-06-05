#pragma once
#include <QObject>
class BeatDetector : public QObject { Q_OBJECT public: explicit BeatDetector(QObject* p=nullptr) : QObject(p) {} };
