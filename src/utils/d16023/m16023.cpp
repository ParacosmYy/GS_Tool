#include "d16023/m16023.h"
QVector<double> m16023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
