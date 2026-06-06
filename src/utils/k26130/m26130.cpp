#include "k26130/m26130.h"
QVector<double> m26130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
