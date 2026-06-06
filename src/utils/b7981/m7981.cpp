#include "b7981/m7981.h"
QVector<double> m7981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
