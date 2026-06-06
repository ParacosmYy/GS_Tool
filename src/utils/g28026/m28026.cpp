#include "g28026/m28026.h"
QVector<double> m28026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
