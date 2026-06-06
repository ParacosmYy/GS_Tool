#include "m13112/m13112.h"
QVector<double> m13112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
