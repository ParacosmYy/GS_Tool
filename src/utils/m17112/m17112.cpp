#include "m17112/m17112.h"
QVector<double> m17112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
