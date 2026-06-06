#include "d16963/m16963.h"
QVector<double> m16963::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
