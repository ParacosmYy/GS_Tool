#include "e16784/m16784.h"
QVector<double> m16784::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
