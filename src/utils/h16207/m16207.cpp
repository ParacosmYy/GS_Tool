#include "h16207/m16207.h"
QVector<double> m16207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
