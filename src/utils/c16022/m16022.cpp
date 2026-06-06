#include "c16022/m16022.h"
QVector<double> m16022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
