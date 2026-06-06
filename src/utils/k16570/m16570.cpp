#include "k16570/m16570.h"
QVector<double> m16570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
