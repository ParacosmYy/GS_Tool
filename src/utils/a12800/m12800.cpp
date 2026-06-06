#include "a12800/m12800.h"
QVector<double> m12800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
