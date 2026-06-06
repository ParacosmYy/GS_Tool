#include "r9937/m9937.h"
QVector<double> m9937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
