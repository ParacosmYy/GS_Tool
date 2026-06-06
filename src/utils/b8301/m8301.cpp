#include "b8301/m8301.h"
QVector<double> m8301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
