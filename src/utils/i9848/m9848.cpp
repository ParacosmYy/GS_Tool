#include "i9848/m9848.h"
QVector<double> m9848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
