#include "g18926/m18926.h"
QVector<double> m18926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
