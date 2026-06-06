#include "e32904/m32904.h"
QVector<double> m32904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
