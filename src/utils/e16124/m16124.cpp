#include "e16124/m16124.h"
QVector<double> m16124::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
