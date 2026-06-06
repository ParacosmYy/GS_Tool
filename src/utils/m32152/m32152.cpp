#include "m32152/m32152.h"
QVector<double> m32152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
