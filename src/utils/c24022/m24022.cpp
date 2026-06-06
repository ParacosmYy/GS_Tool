#include "c24022/m24022.h"
QVector<double> m24022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
