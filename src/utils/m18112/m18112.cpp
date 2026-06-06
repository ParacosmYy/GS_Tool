#include "m18112/m18112.h"
QVector<double> m18112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
