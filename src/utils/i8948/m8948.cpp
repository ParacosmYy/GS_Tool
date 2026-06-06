#include "i8948/m8948.h"
QVector<double> m8948::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
