#include "m34272/m34272.h"
QVector<double> m34272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
