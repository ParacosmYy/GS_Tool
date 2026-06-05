#pragma once
#include <QObject>
class TransientDetect : public QObject { Q_OBJECT public: explicit TransientDetect(QObject* p=nullptr) : QObject(p) {} };
