#include "b18981/m18981.h"
QVector<double> m18981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
