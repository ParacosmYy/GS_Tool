#include "k26630/m26630.h"
QVector<double> m26630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
