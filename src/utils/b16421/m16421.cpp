#include "b16421/m16421.h"
QVector<double> m16421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
