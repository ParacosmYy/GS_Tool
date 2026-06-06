#include "a8600/m8600.h"
QVector<double> m8600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
