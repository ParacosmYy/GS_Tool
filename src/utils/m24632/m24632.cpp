#include "m24632/m24632.h"
QVector<double> m24632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
