#include "b15061/m15061.h"
QVector<double> m15061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
