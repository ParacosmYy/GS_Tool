#include "a32280/m32280.h"
QVector<double> m32280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
