#include "m16152/m16152.h"
QVector<double> m16152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
