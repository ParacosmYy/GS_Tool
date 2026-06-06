#include "m16112/m16112.h"
QVector<double> m16112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
