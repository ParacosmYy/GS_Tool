#include "e25624/m25624.h"
QVector<double> m25624::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
