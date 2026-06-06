#include "g8926/m8926.h"
QVector<double> m8926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
