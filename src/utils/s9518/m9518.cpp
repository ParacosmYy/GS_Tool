#include "s9518/m9518.h"
QVector<double> m9518::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
