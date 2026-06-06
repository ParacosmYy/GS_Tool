#include "s8318/m8318.h"
QVector<double> m8318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
