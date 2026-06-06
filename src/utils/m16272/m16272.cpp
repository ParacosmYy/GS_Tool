#include "m16272/m16272.h"
QVector<double> m16272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
