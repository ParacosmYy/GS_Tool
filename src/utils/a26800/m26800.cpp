#include "a26800/m26800.h"
QVector<double> m26800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
