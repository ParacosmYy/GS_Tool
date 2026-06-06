#include "b16081/m16081.h"
QVector<double> m16081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
