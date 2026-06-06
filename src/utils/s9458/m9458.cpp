#include "s9458/m9458.h"
QVector<double> m9458::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
