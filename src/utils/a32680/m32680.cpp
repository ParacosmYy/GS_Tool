#include "a32680/m32680.h"
QVector<double> m32680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
