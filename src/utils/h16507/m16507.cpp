#include "h16507/m16507.h"
QVector<double> m16507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
