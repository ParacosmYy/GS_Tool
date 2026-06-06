#include "e16084/m16084.h"
QVector<double> m16084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
