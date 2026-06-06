#include "k16370/m16370.h"
QVector<double> m16370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
