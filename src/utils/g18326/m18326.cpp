#include "g18326/m18326.h"
QVector<double> m18326::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
