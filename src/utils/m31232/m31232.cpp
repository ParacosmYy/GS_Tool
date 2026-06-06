#include "m31232/m31232.h"
QVector<double> m31232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
