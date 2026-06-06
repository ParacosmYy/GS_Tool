#include "l17711/m17711.h"
QVector<double> m17711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
