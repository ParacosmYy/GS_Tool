#include "a32380/m32380.h"
QVector<double> m32380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
