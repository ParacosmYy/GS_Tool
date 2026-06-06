#include "e18064/m18064.h"
QVector<double> m18064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
