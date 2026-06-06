#include "l8151/m8151.h"
QVector<double> m8151::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
