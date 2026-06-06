#include "s14018/m14018.h"
QVector<double> m14018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
