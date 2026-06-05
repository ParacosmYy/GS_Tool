#pragma once
#include <QObject>
class LdpcEncoder : public QObject { Q_OBJECT public: explicit LdpcEncoder(QObject* p=nullptr) : QObject(p) {} };
