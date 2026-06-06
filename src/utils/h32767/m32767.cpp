#include "h32767/m32767.h"
QVector<double> m32767::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
