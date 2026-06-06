#include "t16299/m16299.h"
QVector<double> m16299::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
