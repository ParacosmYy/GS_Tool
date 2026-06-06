#include "g28446/m28446.h"
QVector<double> m28446::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
