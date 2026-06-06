#include "m31372/m31372.h"
QVector<double> m31372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
