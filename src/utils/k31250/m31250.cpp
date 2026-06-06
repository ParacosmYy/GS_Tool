#include "k31250/m31250.h"
QVector<double> m31250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
