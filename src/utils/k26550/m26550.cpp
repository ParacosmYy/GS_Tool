#include "k26550/m26550.h"
QVector<double> m26550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
