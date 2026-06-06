#include "c32582/m32582.h"
QVector<double> m32582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
