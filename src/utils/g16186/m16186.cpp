#include "g16186/m16186.h"
QVector<double> m16186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
