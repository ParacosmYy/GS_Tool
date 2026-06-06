#include "i16928/m16928.h"
QVector<double> m16928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
