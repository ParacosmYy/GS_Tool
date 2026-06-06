#include "m27132/m27132.h"
QVector<double> m27132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
