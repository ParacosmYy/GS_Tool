#include "g18086/m18086.h"
QVector<double> m18086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
