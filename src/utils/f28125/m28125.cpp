#include "f28125/m28125.h"
QVector<double> m28125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
