#include "a28480/m28480.h"
QVector<double> m28480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
