#include "d16703/m16703.h"
QVector<double> m16703::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
