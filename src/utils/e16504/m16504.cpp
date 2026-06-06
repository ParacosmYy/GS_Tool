#include "e16504/m16504.h"
QVector<double> m16504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
