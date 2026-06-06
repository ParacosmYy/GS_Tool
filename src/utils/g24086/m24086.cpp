#include "g24086/m24086.h"
QVector<double> m24086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
