#include "m37732/m37732.h"
QVector<double> m37732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
