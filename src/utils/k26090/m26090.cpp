#include "k26090/m26090.h"
QVector<double> m26090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
