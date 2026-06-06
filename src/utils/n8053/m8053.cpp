#include "n8053/m8053.h"
QVector<double> m8053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
