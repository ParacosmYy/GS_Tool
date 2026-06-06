#include "i12848/m12848.h"
QVector<double> m12848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
