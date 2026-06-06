#include "s8218/m8218.h"
QVector<double> m8218::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
