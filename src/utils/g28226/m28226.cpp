#include "g28226/m28226.h"
QVector<double> m28226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
