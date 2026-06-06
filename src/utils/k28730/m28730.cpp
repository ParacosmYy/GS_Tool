#include "k28730/m28730.h"
QVector<double> m28730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
