#include "s25378/m25378.h"
QVector<double> m25378::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
