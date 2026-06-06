#include "c16302/m16302.h"
QVector<double> m16302::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
