#include "a21500/m21500.h"
QVector<double> m21500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
