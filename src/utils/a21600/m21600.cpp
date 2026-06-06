#include "a21600/m21600.h"
QVector<double> m21600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
