#include "a28800/m28800.h"
QVector<double> m28800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
