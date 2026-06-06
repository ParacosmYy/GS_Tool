#include "p18815/m18815.h"
QVector<double> m18815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
