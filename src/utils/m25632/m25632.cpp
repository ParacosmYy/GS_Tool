#include "m25632/m25632.h"
QVector<double> m25632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
