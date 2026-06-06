#include "m24732/m24732.h"
QVector<double> m24732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
