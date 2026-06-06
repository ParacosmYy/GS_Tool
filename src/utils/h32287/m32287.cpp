#include "h32287/m32287.h"
QVector<double> m32287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
