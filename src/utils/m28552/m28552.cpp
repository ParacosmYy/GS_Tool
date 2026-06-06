#include "m28552/m28552.h"
QVector<double> m28552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
