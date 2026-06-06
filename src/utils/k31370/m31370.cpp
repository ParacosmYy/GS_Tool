#include "k31370/m31370.h"
QVector<double> m31370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
