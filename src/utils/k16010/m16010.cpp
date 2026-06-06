#include "k16010/m16010.h"
QVector<double> m16010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
