#include "k32550/m32550.h"
QVector<double> m32550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
