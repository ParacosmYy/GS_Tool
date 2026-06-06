#include "p32835/m32835.h"
QVector<double> m32835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
