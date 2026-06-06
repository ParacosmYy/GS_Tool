#include "h16607/m16607.h"
QVector<double> m16607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
