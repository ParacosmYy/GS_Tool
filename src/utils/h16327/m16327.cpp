#include "h16327/m16327.h"
QVector<double> m16327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
