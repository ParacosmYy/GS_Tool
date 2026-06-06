#include "m9012/m9012.h"
QVector<double> m9012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
