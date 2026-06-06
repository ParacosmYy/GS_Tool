#include "b8981/m8981.h"
QVector<double> m8981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
