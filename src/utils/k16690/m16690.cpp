#include "k16690/m16690.h"
QVector<double> m16690::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
