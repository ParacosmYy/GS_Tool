#include "a32820/m32820.h"
QVector<double> m32820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
