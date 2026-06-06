#include "l13011/m13011.h"
QVector<double> m13011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
