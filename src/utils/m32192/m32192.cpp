#include "m32192/m32192.h"
QVector<double> m32192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
