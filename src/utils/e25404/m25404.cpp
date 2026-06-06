#include "e25404/m25404.h"
QVector<double> m25404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
