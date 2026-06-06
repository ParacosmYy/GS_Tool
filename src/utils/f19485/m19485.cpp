#include "f19485/m19485.h"
QVector<double> m19485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
