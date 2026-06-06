#include "r8817/m8817.h"
QVector<double> m8817::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
