#include "r7937/m7937.h"
QVector<double> m7937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
