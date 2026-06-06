#include "h7867/m7867.h"
QVector<double> m7867::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
