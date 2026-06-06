#include "k28150/m28150.h"
QVector<double> m28150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
