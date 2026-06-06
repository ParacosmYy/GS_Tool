#include "e16804/m16804.h"
QVector<double> m16804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
