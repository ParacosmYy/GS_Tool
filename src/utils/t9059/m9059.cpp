#include "t9059/m9059.h"
QVector<double> m9059::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
