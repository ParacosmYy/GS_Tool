#include "e25104/m25104.h"
QVector<double> m25104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
