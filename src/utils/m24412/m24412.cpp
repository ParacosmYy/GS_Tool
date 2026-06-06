#include "m24412/m24412.h"
QVector<double> m24412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
