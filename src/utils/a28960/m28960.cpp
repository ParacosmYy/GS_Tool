#include "a28960/m28960.h"
QVector<double> m28960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
