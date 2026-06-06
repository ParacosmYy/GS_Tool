#include "m26832/m26832.h"
QVector<double> m26832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
