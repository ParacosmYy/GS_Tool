#include "e35084/m35084.h"
QVector<double> m35084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
