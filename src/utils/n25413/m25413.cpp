#include "n25413/m25413.h"
QVector<double> m25413::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
