#pragma once
#include <QObject>
class SpectralEstimator : public QObject { Q_OBJECT public: explicit SpectralEstimator(QObject* p=nullptr) : QObject(p) {} };
