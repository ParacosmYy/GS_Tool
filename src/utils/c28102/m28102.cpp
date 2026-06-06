#include "c28102/m28102.h"
QVector<double> m28102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
