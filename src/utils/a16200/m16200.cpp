#include "a16200/m16200.h"
QVector<double> m16200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
