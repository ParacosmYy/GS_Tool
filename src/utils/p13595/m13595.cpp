#include "p13595/m13595.h"
QVector<double> m13595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
