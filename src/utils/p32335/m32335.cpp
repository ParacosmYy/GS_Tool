#include "p32335/m32335.h"
QVector<double> m32335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
