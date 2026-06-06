#include "m24492/m24492.h"
QVector<double> m24492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
