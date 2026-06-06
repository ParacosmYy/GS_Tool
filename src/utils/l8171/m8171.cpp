#include "l8171/m8171.h"
QVector<double> m8171::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
