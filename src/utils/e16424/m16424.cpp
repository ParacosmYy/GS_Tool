#include "e16424/m16424.h"
QVector<double> m16424::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
