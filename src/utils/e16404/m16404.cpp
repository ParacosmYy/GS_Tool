#include "e16404/m16404.h"
QVector<double> m16404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
