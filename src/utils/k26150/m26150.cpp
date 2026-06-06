#include "k26150/m26150.h"
QVector<double> m26150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
