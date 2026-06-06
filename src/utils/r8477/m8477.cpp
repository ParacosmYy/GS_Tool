#include "r8477/m8477.h"
QVector<double> m8477::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
