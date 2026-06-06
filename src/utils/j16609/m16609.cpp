#include "j16609/m16609.h"
QVector<double> m16609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
