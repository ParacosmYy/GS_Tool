#include "r8017/m8017.h"
QVector<double> m8017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
