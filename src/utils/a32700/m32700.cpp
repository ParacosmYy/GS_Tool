#include "a32700/m32700.h"
QVector<double> m32700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
