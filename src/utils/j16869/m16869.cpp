#include "j16869/m16869.h"
QVector<double> m16869::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
