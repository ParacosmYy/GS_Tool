#include "g28506/m28506.h"
QVector<double> m28506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
