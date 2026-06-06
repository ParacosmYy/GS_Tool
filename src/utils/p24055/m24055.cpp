#include "p24055/m24055.h"
QVector<double> m24055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
