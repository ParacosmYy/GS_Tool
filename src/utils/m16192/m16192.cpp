#include "m16192/m16192.h"
QVector<double> m16192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
