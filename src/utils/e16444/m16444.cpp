#include "e16444/m16444.h"
QVector<double> m16444::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
