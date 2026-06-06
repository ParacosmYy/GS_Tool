#include "i31408/m31408.h"
QVector<double> m31408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
