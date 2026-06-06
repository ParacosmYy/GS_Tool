#include "k32190/m32190.h"
QVector<double> m32190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
