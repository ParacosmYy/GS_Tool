#include "d16323/m16323.h"
QVector<double> m16323::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
