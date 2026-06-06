#include "m37552/m37552.h"
QVector<double> m37552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
