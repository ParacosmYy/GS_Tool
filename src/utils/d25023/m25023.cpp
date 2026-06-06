#include "d25023/m25023.h"
QVector<double> m25023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
