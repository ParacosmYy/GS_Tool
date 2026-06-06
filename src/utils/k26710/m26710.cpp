#include "k26710/m26710.h"
QVector<double> m26710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
