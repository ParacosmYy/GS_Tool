#include "f28405/m28405.h"
QVector<double> m28405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
