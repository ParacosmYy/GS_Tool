#include "k32410/m32410.h"
QVector<double> m32410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
