#include "k32650/m32650.h"
QVector<double> m32650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
