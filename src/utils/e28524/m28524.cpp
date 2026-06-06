#include "e28524/m28524.h"
QVector<double> m28524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
