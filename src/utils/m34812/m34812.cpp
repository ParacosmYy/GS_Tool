#include "m34812/m34812.h"
QVector<double> m34812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
