#include "g9846/m9846.h"
QVector<double> m9846::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
