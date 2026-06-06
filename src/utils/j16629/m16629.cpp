#include "j16629/m16629.h"
QVector<double> m16629::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
