#include "p32595/m32595.h"
QVector<double> m32595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
