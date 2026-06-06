#include "p32115/m32115.h"
QVector<double> m32115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
