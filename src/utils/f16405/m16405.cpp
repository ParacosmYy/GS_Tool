#include "f16405/m16405.h"
QVector<double> m16405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
