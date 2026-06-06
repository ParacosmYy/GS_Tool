#include "a16600/m16600.h"
QVector<double> m16600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
