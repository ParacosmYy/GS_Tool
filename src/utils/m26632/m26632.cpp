#include "m26632/m26632.h"
QVector<double> m26632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
