#include "m9652/m9652.h"
QVector<double> m9652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
