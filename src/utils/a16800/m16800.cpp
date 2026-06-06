#include "a16800/m16800.h"
QVector<double> m16800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
