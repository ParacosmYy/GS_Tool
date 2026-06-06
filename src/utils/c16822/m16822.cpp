#include "c16822/m16822.h"
QVector<double> m16822::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
