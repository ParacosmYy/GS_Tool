#include "k32390/m32390.h"
QVector<double> m32390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
