#include "e16544/m16544.h"
QVector<double> m16544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
