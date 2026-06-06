#include "a32620/m32620.h"
QVector<double> m32620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
