#include "m34012/m34012.h"
QVector<double> m34012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
