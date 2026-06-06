#include "l16751/m16751.h"
QVector<double> m16751::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
