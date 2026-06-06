#include "k26610/m26610.h"
QVector<double> m26610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
