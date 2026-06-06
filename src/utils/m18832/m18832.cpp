#include "m18832/m18832.h"
QVector<double> m18832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
