#include "c16622/m16622.h"
QVector<double> m16622::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
