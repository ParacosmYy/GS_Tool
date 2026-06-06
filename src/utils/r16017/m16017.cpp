#include "r16017/m16017.h"
QVector<double> m16017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
