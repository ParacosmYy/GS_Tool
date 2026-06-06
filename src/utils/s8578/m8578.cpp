#include "s8578/m8578.h"
QVector<double> m8578::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
