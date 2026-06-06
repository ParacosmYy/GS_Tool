#include "s16018/m16018.h"
QVector<double> m16018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
