#include "a8320/m8320.h"
QVector<double> m8320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
