#include "a16880/m16880.h"
QVector<double> m16880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
