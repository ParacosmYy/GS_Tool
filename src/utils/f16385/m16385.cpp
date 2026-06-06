#include "f16385/m16385.h"
QVector<double> m16385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
