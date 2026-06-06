#include "m24772/m24772.h"
QVector<double> m24772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
