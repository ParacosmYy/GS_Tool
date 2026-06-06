#include "m16132/m16132.h"
QVector<double> m16132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
