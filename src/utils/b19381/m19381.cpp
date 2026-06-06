#include "b19381/m19381.h"
QVector<double> m19381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
