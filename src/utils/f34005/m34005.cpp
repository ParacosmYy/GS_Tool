#include "f34005/m34005.h"
QVector<double> m34005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
