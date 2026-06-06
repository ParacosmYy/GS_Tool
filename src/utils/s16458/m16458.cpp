#include "s16458/m16458.h"
QVector<double> m16458::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
