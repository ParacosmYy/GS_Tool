#include "d16563/m16563.h"
QVector<double> m16563::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
