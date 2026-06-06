#include "k26330/m26330.h"
QVector<double> m26330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
