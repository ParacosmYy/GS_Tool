#include "k16670/m16670.h"
QVector<double> m16670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
