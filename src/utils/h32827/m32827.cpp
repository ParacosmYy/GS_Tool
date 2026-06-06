#include "h32827/m32827.h"
QVector<double> m32827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
