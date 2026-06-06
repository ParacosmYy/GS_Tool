#include "p16215/m16215.h"
QVector<double> m16215::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
