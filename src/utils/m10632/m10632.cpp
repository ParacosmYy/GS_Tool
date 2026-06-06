#include "m10632/m10632.h"
QVector<double> m10632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
