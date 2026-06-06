#include "m32632/m32632.h"
QVector<double> m32632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
