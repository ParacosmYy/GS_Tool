#include "e16244/m16244.h"
QVector<double> m16244::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
