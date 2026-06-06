#include "b16621/m16621.h"
QVector<double> m16621::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
