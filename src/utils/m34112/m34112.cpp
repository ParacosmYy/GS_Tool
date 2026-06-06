#include "m34112/m34112.h"
QVector<double> m34112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
