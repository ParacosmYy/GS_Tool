#include "e8624/m8624.h"
QVector<double> m8624::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
