#include "a13280/m13280.h"
QVector<double> m13280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
