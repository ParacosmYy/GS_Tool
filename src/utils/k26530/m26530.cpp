#include "k26530/m26530.h"
QVector<double> m26530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
