#include "d16103/m16103.h"
QVector<double> m16103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
