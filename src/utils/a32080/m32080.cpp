#include "a32080/m32080.h"
QVector<double> m32080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
