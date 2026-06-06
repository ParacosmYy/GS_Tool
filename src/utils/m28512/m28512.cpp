#include "m28512/m28512.h"
QVector<double> m28512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
