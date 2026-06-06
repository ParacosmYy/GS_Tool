#include "a21200/m21200.h"
QVector<double> m21200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
