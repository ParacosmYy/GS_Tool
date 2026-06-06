#include "b18281/m18281.h"
QVector<double> m18281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
