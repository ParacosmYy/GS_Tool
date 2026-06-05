#pragma once
#include <QObject>
class DynamicCompressor : public QObject { Q_OBJECT public: explicit DynamicCompressor(QObject* p=nullptr) : QObject(p) {} };
