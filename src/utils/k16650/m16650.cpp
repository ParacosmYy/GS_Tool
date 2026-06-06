#include "k16650/m16650.h"
QVector<double> m16650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
