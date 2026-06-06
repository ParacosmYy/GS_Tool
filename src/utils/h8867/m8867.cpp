#include "h8867/m8867.h"
QVector<double> m8867::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
