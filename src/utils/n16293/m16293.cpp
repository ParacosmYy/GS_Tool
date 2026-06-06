#include "n16293/m16293.h"
QVector<double> m16293::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
