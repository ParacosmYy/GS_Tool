#include "p32575/m32575.h"
QVector<double> m32575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
