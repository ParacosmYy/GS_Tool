#include "a14600/m14600.h"
QVector<double> m14600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
