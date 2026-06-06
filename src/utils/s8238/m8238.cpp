#include "s8238/m8238.h"
QVector<double> m8238::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
