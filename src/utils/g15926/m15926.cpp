#include "g15926/m15926.h"
QVector<double> m15926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
