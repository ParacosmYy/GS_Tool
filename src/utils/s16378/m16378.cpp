#include "s16378/m16378.h"
QVector<double> m16378::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
