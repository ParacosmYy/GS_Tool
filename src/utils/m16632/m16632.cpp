#include "m16632/m16632.h"
QVector<double> m16632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
