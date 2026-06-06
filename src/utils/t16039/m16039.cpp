#include "t16039/m16039.h"
QVector<double> m16039::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
