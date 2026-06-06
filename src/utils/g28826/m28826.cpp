#include "g28826/m28826.h"
QVector<double> m28826::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
