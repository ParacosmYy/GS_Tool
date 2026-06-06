#include "j16789/m16789.h"
QVector<double> m16789::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
