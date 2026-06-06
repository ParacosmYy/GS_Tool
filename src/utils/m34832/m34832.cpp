#include "m34832/m34832.h"
QVector<double> m34832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
