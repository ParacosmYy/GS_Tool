#include "e16524/m16524.h"
QVector<double> m16524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
