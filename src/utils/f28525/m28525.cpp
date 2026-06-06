#include "f28525/m28525.h"
QVector<double> m28525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
