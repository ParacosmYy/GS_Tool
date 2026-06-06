#include "g28346/m28346.h"
QVector<double> m28346::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
