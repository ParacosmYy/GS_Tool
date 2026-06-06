#include "m8632/m8632.h"
QVector<double> m8632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
