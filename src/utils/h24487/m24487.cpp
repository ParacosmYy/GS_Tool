#include "h24487/m24487.h"
QVector<double> m24487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
