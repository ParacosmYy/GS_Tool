#include "n8413/m8413.h"
QVector<double> m8413::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
