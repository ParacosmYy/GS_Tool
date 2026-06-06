#include "l16131/m16131.h"
QVector<double> m16131::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
