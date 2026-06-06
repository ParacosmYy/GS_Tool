#include "i24448/m24448.h"
QVector<double> m24448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
