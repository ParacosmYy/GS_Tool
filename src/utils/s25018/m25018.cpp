#include "s25018/m25018.h"
QVector<double> m25018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
