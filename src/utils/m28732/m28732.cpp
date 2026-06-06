#include "m28732/m28732.h"
QVector<double> m28732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
