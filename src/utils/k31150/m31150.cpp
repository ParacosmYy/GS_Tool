#include "k31150/m31150.h"
QVector<double> m31150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
