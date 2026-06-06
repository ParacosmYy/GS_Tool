#include "q32576/m32576.h"
QVector<double> m32576::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
