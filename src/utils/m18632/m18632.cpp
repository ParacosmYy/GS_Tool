#include "m18632/m18632.h"
QVector<double> m18632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
