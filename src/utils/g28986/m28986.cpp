#include "g28986/m28986.h"
QVector<double> m28986::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
