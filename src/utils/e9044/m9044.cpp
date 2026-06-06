#include "e9044/m9044.h"
QVector<double> m9044::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
