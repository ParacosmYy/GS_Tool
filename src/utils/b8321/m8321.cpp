#include "b8321/m8321.h"
QVector<double> m8321::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
