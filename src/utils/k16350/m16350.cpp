#include "k16350/m16350.h"
QVector<double> m16350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
