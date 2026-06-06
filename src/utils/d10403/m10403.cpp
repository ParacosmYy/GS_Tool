#include "d10403/m10403.h"
QVector<double> m10403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
