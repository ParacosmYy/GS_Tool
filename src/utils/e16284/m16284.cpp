#include "e16284/m16284.h"
QVector<double> m16284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
