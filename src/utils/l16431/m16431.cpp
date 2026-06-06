#include "l16431/m16431.h"
QVector<double> m16431::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
