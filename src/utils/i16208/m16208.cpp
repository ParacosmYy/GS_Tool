#include "i16208/m16208.h"
QVector<double> m16208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
