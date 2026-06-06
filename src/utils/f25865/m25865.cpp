#include "f25865/m25865.h"
QVector<double> m25865::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
