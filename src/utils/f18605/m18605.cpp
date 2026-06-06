#include "f18605/m18605.h"
QVector<double> m18605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
