#include "k16050/m16050.h"
QVector<double> m16050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
