#include "e28104/m28104.h"
QVector<double> m28104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
