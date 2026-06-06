#include "c24602/m24602.h"
QVector<double> m24602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
