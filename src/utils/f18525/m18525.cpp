#include "f18525/m18525.h"
QVector<double> m18525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
