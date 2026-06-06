#include "p32255/m32255.h"
QVector<double> m32255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
