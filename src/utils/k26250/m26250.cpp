#include "k26250/m26250.h"
QVector<double> m26250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
