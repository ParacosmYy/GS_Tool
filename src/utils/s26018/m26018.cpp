#include "s26018/m26018.h"
QVector<double> m26018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
