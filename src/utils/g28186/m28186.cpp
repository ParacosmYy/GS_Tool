#include "g28186/m28186.h"
QVector<double> m28186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
