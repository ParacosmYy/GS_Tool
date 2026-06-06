#include "b18321/m18321.h"
QVector<double> m18321::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
