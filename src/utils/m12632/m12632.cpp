#include "m12632/m12632.h"
QVector<double> m12632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
