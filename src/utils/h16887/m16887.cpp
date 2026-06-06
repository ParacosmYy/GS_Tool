#include "h16887/m16887.h"
QVector<double> m16887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
