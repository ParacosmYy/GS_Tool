#include "e28084/m28084.h"
QVector<double> m28084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
