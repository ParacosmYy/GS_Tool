#include "n16413/m16413.h"
QVector<double> m16413::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
