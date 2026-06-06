#include "a18820/m18820.h"
QVector<double> m18820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
