#include "j16709/m16709.h"
QVector<double> m16709::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
