#include "r26377/m26377.h"
QVector<double> m26377::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
