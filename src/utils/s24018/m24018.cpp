#include "s24018/m24018.h"
QVector<double> m24018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
