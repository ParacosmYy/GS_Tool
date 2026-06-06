#include "r23937/m23937.h"
QVector<double> m23937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
