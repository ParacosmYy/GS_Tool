#include "t28219/m28219.h"
QVector<double> m28219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
