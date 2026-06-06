#include "k16330/m16330.h"
QVector<double> m16330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
