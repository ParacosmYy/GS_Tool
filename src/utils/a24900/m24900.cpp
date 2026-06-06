#include "a24900/m24900.h"
QVector<double> m24900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
