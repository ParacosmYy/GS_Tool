#pragma once
#include <QObject>
class GraphPartition : public QObject { Q_OBJECT public: explicit GraphPartition(QObject* p=nullptr) : QObject(p) {} };
