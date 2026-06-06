#include "l16071/m16071.h"
QVector<double> m16071::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
