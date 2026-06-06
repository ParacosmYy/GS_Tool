#include "q32816/m32816.h"
QVector<double> m32816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
