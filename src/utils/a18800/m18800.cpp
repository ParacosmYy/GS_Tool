#include "a18800/m18800.h"
QVector<double> m18800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
