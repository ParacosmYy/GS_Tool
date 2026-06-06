#include "h16427/m16427.h"
QVector<double> m16427::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
