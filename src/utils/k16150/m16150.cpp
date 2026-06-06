#include "k16150/m16150.h"
QVector<double> m16150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
