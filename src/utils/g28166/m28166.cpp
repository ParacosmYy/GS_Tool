#include "g28166/m28166.h"
QVector<double> m28166::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
