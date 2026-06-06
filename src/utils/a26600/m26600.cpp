#include "a26600/m26600.h"
QVector<double> m26600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
