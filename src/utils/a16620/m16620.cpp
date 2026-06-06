#include "a16620/m16620.h"
QVector<double> m16620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
