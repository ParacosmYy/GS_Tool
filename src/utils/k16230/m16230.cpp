#include "k16230/m16230.h"
QVector<double> m16230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
