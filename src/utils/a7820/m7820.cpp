#include "a7820/m7820.h"
QVector<double> m7820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
