#pragma once
#include <QObject>
class HierClassifier : public QObject { Q_OBJECT public: explicit HierClassifier(QObject* p=nullptr) : QObject(p) {} };
