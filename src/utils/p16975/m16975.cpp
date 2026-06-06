#include "p16975/m16975.h"
QVector<double> m16975::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
