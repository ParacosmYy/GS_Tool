#include "r8937/m8937.h"
QVector<double> m8937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
