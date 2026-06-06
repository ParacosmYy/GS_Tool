#include "s28018/m28018.h"
QVector<double> m28018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
