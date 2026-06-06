#include "m24232/m24232.h"
QVector<double> m24232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
