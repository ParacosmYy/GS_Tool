#include "a16640/m16640.h"
QVector<double> m16640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
