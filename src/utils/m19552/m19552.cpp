#include "m19552/m19552.h"
QVector<double> m19552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
