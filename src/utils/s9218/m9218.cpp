#include "s9218/m9218.h"
QVector<double> m9218::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
