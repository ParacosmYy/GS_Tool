#include "i16328/m16328.h"
QVector<double> m16328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
