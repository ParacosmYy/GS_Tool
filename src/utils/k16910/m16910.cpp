#include "k16910/m16910.h"
QVector<double> m16910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
