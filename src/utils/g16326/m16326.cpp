#include "g16326/m16326.h"
QVector<double> m16326::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
