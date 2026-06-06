#include "a28900/m28900.h"
QVector<double> m28900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
