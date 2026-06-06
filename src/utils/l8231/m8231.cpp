#include "l8231/m8231.h"
QVector<double> m8231::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
