#include "a16380/m16380.h"
QVector<double> m16380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
