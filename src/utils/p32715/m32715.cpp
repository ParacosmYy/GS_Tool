#include "p32715/m32715.h"
QVector<double> m32715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
