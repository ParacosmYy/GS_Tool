#include "e8004/m8004.h"
QVector<double> m8004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
