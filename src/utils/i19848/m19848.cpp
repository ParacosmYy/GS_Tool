#include "i19848/m19848.h"
QVector<double> m19848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
