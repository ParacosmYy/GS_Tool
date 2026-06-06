#include "k7950/m7950.h"
QVector<double> m7950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
