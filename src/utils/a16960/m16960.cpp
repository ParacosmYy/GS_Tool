#include "a16960/m16960.h"
QVector<double> m16960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
