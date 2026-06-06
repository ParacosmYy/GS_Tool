#include "r16937/m16937.h"
QVector<double> m16937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
