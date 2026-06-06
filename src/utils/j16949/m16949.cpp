#include "j16949/m16949.h"
QVector<double> m16949::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
