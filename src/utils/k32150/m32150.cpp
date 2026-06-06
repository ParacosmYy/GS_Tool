#include "k32150/m32150.h"
QVector<double> m32150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
