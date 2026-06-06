#include "d25603/m25603.h"
QVector<double> m25603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
