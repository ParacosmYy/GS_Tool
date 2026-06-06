#include "k26050/m26050.h"
QVector<double> m26050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
