#include "m16352/m16352.h"
QVector<double> m16352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
