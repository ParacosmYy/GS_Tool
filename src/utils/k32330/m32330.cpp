#include "k32330/m32330.h"
QVector<double> m32330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
