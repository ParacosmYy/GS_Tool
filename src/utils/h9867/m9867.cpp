#include "h9867/m9867.h"
QVector<double> m9867::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
