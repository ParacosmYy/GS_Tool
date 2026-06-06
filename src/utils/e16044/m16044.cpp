#include "e16044/m16044.h"
QVector<double> m16044::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
