#include "f25485/m25485.h"
QVector<double> m25485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
