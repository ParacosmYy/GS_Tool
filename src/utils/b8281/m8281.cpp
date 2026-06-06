#include "b8281/m8281.h"
QVector<double> m8281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
