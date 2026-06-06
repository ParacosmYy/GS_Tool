#include "p24115/m24115.h"
QVector<double> m24115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
