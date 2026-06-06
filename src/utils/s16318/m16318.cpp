#include "s16318/m16318.h"
QVector<double> m16318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
