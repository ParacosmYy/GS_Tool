#include "e8764/m8764.h"
QVector<double> m8764::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
