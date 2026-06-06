#include "k26010/m26010.h"
QVector<double> m26010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
