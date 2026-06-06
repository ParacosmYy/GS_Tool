#include "r19937/m19937.h"
QVector<double> m19937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
