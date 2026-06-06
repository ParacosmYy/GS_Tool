#include "e16104/m16104.h"
QVector<double> m16104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
