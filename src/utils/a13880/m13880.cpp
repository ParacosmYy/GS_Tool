#include "a13880/m13880.h"
QVector<double> m13880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
