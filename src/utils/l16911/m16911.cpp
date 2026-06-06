#include "l16911/m16911.h"
QVector<double> m16911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
