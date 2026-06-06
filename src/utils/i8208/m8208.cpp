#include "i8208/m8208.h"
QVector<double> m8208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
