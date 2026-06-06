#include "p32555/m32555.h"
QVector<double> m32555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
